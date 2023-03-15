/**
 * @file fastq_parse.cpp
 * @brief SharQ application
 *
 */

/*  $Id: fastq_parse.cpp 637208 2021-09-08 21:30:39Z shkeda $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Author:  Andrei Shkeda
* File Description: SharQ application
*
* ===========================================================================
*/
// command line
#include "CLI11.hpp"
// logging
#include <spdlog/fmt/fmt.h>

#include "version.h"
#include "fastq_error.hpp"
#include "fastq_parser.hpp"
#include "fastq_writer.hpp"

#include <json.hpp>

#if __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::experimental::filesystem;
#endif

using json = nlohmann::json;

#define LOCALDEBUG

/**
 * @brief SharQ application class
 *
 * After processing the command line arguments and input files,
 * the application builds a digest (json data structure) for the first 500K spots.
 *
 * Digest captures the payload properties as well as identifies quality score encoding.
 *
 * Parser and writer are set up using the digest data
 *
 * Witer's output (stdout) is expected to be piped in general_loader application.
 *
 * --debug parameter can be used to send the output to stdout
 */
class CFastqParseApp
{
public:
    int AppMain(int argc, const char* argv[]);
private:
    int Run();
    int xRun();
    int xRunDigest();

    void xSetupInput();
    void xSetupOutput();

    // check consistency of digest data
    // verify the same platform, the same platform
    void xProcessDigest(json& data);

    void xBreakdownOutput();
    void xCheckInputFiles(vector<string>& files);

    void xReportTelemetry();
    void xCheckErrorLimits(fastq_error& e);

    string mDestination;                ///< path to sra archive
    bool mDebug{false};                 ///< Debug mode
    bool mNoTimeStamp{false};           ///< No time stamp in debug mode
    vector<char> mReadTypes;            ///< ReadType paramter value
    using TInputFiles = vector<string>;
    vector<TInputFiles> mInputBatches;  ///< List of input batches
    bool mDiscardNames{false};          ///< If set spot names are not written in the db, the same effect as mNameColumn = 'NONE'
    bool mAllowEarlyFileEnd{false};     ///< Flag to continue if one of the streams ends
    int mQuality{-1};                   ///< quality score interpretation (0, 33, 64)
    int mDigest{0};                     ///< Number of digest lines to produce
    string mTelemetryFile;              ///< Telemetry report file name
    string mSpotFile;                   ///< Spot_name file, optional request to serialize  all spot names
    string mNameColumn;                 ///< NAME column's name, ('NONE', 'NAME', 'RAW_NAME')
    string mOutputFile;                 ///< Outut file name - not currently used
    ostream* mpOutStr{nullptr};         ///< Output stream pointer  = not currently used
    shared_ptr<fastq_writer> m_writer;  ///< FASTQ writer
    json  mReport;                      ///< Telemetry report
    uint32_t mMaxErrCount{100};         ///< Maximum numbers of errors allowed when parsing reads
    uint32_t mErrorCount{0};            ///< Global error counter
    set<int> mErrorSet = { 100, 110, 111, 120, 130, 140, 160, 190}; ///< Error codes that will be allowed up to mMaxErrCount
};


void s_AddReadPairBatch(vector<string>& batch, vector<vector<string>>& out)
{
    if (batch.empty())
        return;
    if (out.empty()) {
        out.resize(batch.size());
    } else if (batch.size() != out.size()) {
        throw fastq_error(10, "Invalid command line parameters, inconsistent number of read pairs");
    }

    for (size_t i = 0; i < batch.size(); ++i) {
        out[i].push_back(move(batch[i]));
    }
}

static
void s_split(const string& str, vector<string>& out, char c = ',')
{
    if (str.empty())
        return;
    auto begin = str.begin();
    auto end  = str.end();
    while (begin != end) {
        auto it = begin;
        while (it != end && *it != c) ++it;
        out.emplace_back(begin, it);
        if (it == end)
            break;
        begin = ++it;
    }
}
//  ----------------------------------------------------------------------------
int CFastqParseApp::AppMain(int argc, const char* argv[])
{
    int ret_code = 0;
    spdlog::stopwatch stop_watch;

    try {
        CLI::App app{"SharQ"};

        mOutputFile.clear();
        mDestination = "sra.out";

        app.set_version_flag("--version,-V", SHARQ_VERSION);

        app.add_option("--output", mDestination, "Output archive path");

        string platform;
        app.add_option("--platform", platform, "Optional platform");

        string read_types;
        app.add_option("--readTypes", read_types, "file read types <B|T(s)>");

        app.add_flag("--useAndDiscardNames", mDiscardNames, "Discard file names");

        app.add_flag("--allowEarlyFileEnd", mAllowEarlyFileEnd, "Complete load at early end of one of the files");

        bool print_errors = false;
        app.add_flag("--help_errors,--help-errors", print_errors, "Print error codes and descriptions");

        app.add_option("--name-column", mNameColumn, "Database name for NAME column")
            ->default_str("NAME")
            ->default_val("NAME")
            ->excludes("--useAndDiscardNames")
            ->check(CLI::IsMember({"NONE", "NAME", "RAW_NAME"}));

        mQuality = -1;
        app.add_option("--quality,-q", mQuality, "Interpretation of ascii quality")
            ->check(CLI::IsMember({0, 33, 64}));

        mDigest = 0;
        app.add_flag("--digest{500000}", mDigest, "Report summary of input data (set optional value to indicate the number of spots to analyze)");

        mTelemetryFile.clear();
        app.add_option("--telemetry,-t", mTelemetryFile, "Telemetry report file");

        vector<string> read_pairs(4);
        app.add_option("--read1PairFiles", read_pairs[0], "Read 1 files");
        app.add_option("--read2PairFiles", read_pairs[1], "Read 2 files");
        app.add_option("--read3PairFiles", read_pairs[2], "Read 3 files");
        app.add_option("--read4PairFiles", read_pairs[3], "Read 4 files");
        app.add_option("--max-err-count", mMaxErrCount, "Maximum number of errors allowed")
            ->default_val(100)
            ->check(CLI::Range(uint32_t(0), numeric_limits<uint32_t>::max()));

        vector<string> input_files;
        app.add_option("files", input_files, "FastQ files to parse");

        auto opt = app.add_option_group("Debugging options");
        mNoTimeStamp = false;
        opt->add_flag("--no-timestamp", mNoTimeStamp, "No time stamp in debug mode");

        string log_level = "info";
        opt->add_option("--log-level", log_level, "Log level")
            ->default_val("info")
            ->check(CLI::IsMember({"trace", "debug", "info", "warning", "error"}));

        string hash_file;
        opt->add_option("--hash", hash_file, "Check hash file");
        opt->add_option("--spot_file", mSpotFile, "Save spot names");
        opt->add_flag("--debug", mDebug, "Debug mode");

        mReport["version"] = SHARQ_VERSION;
        // save cmd args
        for (int i = 1; i < argc; ++i)
            mReport["args"].push_back(argv[i]);

        CLI11_PARSE(app, argc, argv);
        if (print_errors) {
            fastq_error::print_error_codes(cout);
            return 0;
        }
        if (mDigest < 0)
            mDigest = -1;
        if (mDigest != 0)
            spdlog::set_level(spdlog::level::from_str("error"));
        else
            spdlog::set_level(spdlog::level::from_str(log_level));

        if (!hash_file.empty()) {
            check_hash_file(hash_file);
            return 0;
        }
        vector<string> options2log = {"--platform", "--readTypes", "--useAndDiscardNames", "--allowEarlyFileEnd", "--name-column", "--quality"};
        for (const auto& opt_name : options2log) {
            auto opt = app.get_option(opt_name);
            if (opt && *opt)
                mReport[opt_name] = opt->as<string>();
        }

        if (mDiscardNames)
            mNameColumn = "NONE";

        xSetupOutput();
        if (mDigest == 0) {
            if (mDebug) {
                m_writer = make_shared<fastq_writer>();
                if (mNoTimeStamp)
                    spdlog::set_pattern("[%l] %v");

            } else {
                spdlog::set_pattern("%v");
                m_writer = make_shared<fastq_writer_vdb>(*mpOutStr);
            }
        }
        if (read_types.find_first_not_of("TBA") != string::npos)
            throw fastq_error(150, "Invalid --readTypes values '{}'", read_types);

        copy(read_types.begin(), read_types.end(), back_inserter(mReadTypes));

        if (!read_pairs[0].empty()) {
            if (mDigest == 0 && mReadTypes.empty())
                throw fastq_error(20, "No readTypes provided");
            for (auto p : read_pairs) {
                vector<string> b;
                s_split(p, b);
                xCheckInputFiles(b);
                s_AddReadPairBatch(b, mInputBatches);
            }
        } else {
            if (input_files.empty()) {
                mInputBatches.push_back({"-"});
            } else {
                stable_sort(input_files.begin(), input_files.end());
                xCheckInputFiles(input_files);
                fastq_reader::cluster_files(input_files, mInputBatches);
            }
        }
        ret_code = Run();
    } catch (fastq_error& e) {
        spdlog::error(e.Message());
        mReport["error"] = e.Message();
        ret_code = 1;
    } catch(std::exception const& e) {
        string error = fmt::format("[code:0] Runtime error: {}", e.what());
        spdlog::error(error);
        mReport["error"] = error;
        ret_code = 1;
    }

    if (mNoTimeStamp == false)
        mReport["exec_time"] =  ceil(stop_watch.elapsed().count() * 100.0) / 100.0;

    xReportTelemetry();
    return ret_code;
}

void CFastqParseApp::xReportTelemetry()
{
    if (mTelemetryFile.empty())
        return;
    try {
        ofstream f(mTelemetryFile.c_str(), ios::out);
        f << mReport.dump(4, ' ', true) << endl;
    } catch(std::exception const& e) {
        spdlog::error("[code:0] Runtime error: {}", e.what());
    }
}


void CFastqParseApp::xCheckInputFiles(vector<string>& files)
{
    for (auto& f : files) {
        if (f == "-" || fs::exists(f)) continue;
        bool not_found = true;
        auto ext = fs::path(f).extension();
        if (ext != ".gz" && ext != ".bz2") {
            if (fs::exists(f + ".gz")) {
                spdlog::debug("File '{}': .gz extension added", f);
                f += ".gz";
                not_found = false;
            } else if (fs::exists(f + ".bz2")) {
                spdlog::debug("File '{}': .bz2 extension added", f);
                f += ".bz2";
                not_found = false;
            }
        } else if (ext == ".gz" || ext == ".bz2") {
            auto new_fn = f.substr(0, f.size() - ext.string().size());
            if (fs::exists(new_fn)) {
                spdlog::debug("File '{}': {} extension ignored", f, ext.string());
                f = new_fn;
                not_found = false;
            }

        }
        if (not_found)
            throw fastq_error(40, "File '{}' does not exist", f);
    }
}

//  ----------------------------------------------------------------------------
int CFastqParseApp::Run()
{
    int retStatus = mDigest != 0 ? xRunDigest() : xRun();
    xBreakdownOutput();
    return retStatus;
}


void CFastqParseApp::xProcessDigest(json& data)
{
    assert(data.contains("groups"));
    assert(data["groups"].front().contains("files"));
    const auto& first = data["groups"].front()["files"].front();
    if (first["platform_code"].size() > 1)
        throw fastq_error(70, "Input file has data from multiple platforms ({} != {})", first["platform_code"][0], first["platform_code"][1]);
    bool is10x = data["groups"].front()["is_10x"];
    int platform = first["platform_code"].front();
    int total_reads = 0;
    for (auto& gr : data["groups"]) {
        int max_reads = 0;
        int group_reads = 0;
        if (gr["is_10x"] != is10x)
            throw fastq_error(80);// "Inconsistent submission: 10x submissions are mixed with different types.");

        auto& files = gr["files"];
        for (auto& f : files) {
            if (mQuality != -1)
                f["quality_encoding"] = mQuality; // Override quality
            if (f["platform_code"].size() > 1)
                throw fastq_error(70, "Input file has data from multiple platforms ({} != {})", f["platform_code"][0], f["platform_code"][1]);
           if (platform != f["platform_code"].front())
                throw fastq_error(70, "Input files have deflines from different platforms ({} != {})", platform, int(f["platform_code"].front()));
            max_reads = max<int>(max_reads, f["max_reads"]);
            group_reads += (int)f["max_reads"];

            // if read types are specified (mix of B and T)
            // and the reads in an interleaved file don't have read numbers
            // and orphans are present
            // then sharq should fail.
            if (!mReadTypes.empty() && max_reads > 1 && f["has_orphans"] && f["readNums"].empty())
                throw fastq_error(190); // "Usupported interleaved file with orphans"
        }

        // non10x, non interleaved files
        // sort by readNumber
        if (is10x == false && max_reads == 1) {
            sort(files.begin(), files.end(), [](const auto& d1, const auto& d2){
                string v1 = d1["readNums"].empty() ? "" : d1["readNums"].front();
                string v2 = d2["readNums"].empty() ? "" : d2["readNums"].front();
                return v1 < v2;
            });
        }
        if (!mReadTypes.empty()) {
            if ((int)mReadTypes.size() != group_reads)
                throw fastq_error(30, "readTypes number should match the number of reads ({})", group_reads);
        }
        total_reads = max<int>(group_reads, total_reads);
        gr["total_reads"] = total_reads;
    }

    if (mReadTypes.empty()) {
        //auto num_files = data["groups"].front().size();
        if (is10x)
            mReadTypes.resize(total_reads, 'A');
        else if (total_reads < 3) {
            mReadTypes.resize(total_reads, 'B');
        } else {
            throw fastq_error(20); // "No readTypes provided");
        }
    }

    for (auto& gr : data["groups"]) {
        auto& files = gr["files"];
        int total_reads = gr["total_reads"];
        if ((int)mReadTypes.size() < total_reads)
            throw fastq_error(30, "readTypes number should match the number of reads");
        int j = 0;
        // assign read types for each file
        for (auto& f : files) {
            int num_reads = f["max_reads"];
            while (num_reads) {
                f["readType"].push_back(char(mReadTypes[j]));
                --num_reads;
                ++j;
            }
        }
    }

    switch ((int)first["quality_encoding"]) {
    case 0:
        m_writer->set_attr("quality_expression", "(INSDC:quality:phred)QUALITY");
        break;
    case 33:
        m_writer->set_attr("quality_expression", "(INSDC:quality:text:phred_33)QUALITY");
        break;
    case 64:
        m_writer->set_attr("quality_expression", "(INSDC:quality:text:phred_64)QUALITY");
        break;
    default:
        throw runtime_error("Invaid quality encoding");
    }
    m_writer->set_attr("platform", to_string(first["platform_code"].front()));

}


//  -----------------------------------------------------------------------------
int CFastqParseApp::xRunDigest()
{
    if (mInputBatches.empty())
        return 1;
    spdlog::set_level(spdlog::level::from_str("off"));
    json j;
    string error;
    try {
        get_digest(j, mInputBatches, [this](fastq_error& e) { CFastqParseApp::xCheckErrorLimits(e);}, mDigest);
    } catch (fastq_error& e) {
        error = e.Message();
    } catch(std::exception const& e) {
        error = fmt::format("[code:0] Runtime error: {}", e.what());
    }
    if (!error.empty()) {
        // remove special character if any
        error.erase(remove_if(error.begin(), error.end(),[](char c) { return !isprint(c); }), error.end());
        j["error"] = error;
    }

    cout << j.dump(4, ' ', true) << endl;
    return 0;
}

int CFastqParseApp::xRun()
{
    if (mInputBatches.empty())
        return 1;
    fastq_parser<fastq_writer> parser(m_writer);
    if (!mDebug)
        parser.set_spot_file(mSpotFile);
    parser.set_allow_early_end(mAllowEarlyFileEnd);
    json data;
    get_digest(data, mInputBatches, [this](fastq_error& e) { CFastqParseApp::xCheckErrorLimits(e);});
    xProcessDigest(data);
    mErrorCount = 0; //Reset error counts after initial digets

    m_writer->set_attr("name_column", mNameColumn);
    m_writer->set_attr("destination", mDestination);
    m_writer->set_attr("version", SHARQ_VERSION);

    m_writer->open();
    auto err_checker = [this](fastq_error& e) { CFastqParseApp::xCheckErrorLimits(e);};
    for (auto& group : data["groups"]) {
        parser.set_readers(group);
        if (!group["files"].empty()) {
            switch ((int)group["files"].front()["quality_encoding"]) {
                case 0:
                    parser.parse<validator_options<eNumeric, -5, 40>>(err_checker);
                    break;
                case 33:
                    parser.parse<validator_options<ePhred, 33, 126>>(err_checker);
                    break;
                case 64:
                    parser.parse<validator_options<ePhred, 64, 126>>(err_checker);
                    break;
                default:
                    throw runtime_error("Invaid quality encoding");
            }
        }
    }
    spdlog::stopwatch sw;
    parser.check_duplicates();
    if (mNoTimeStamp == false)
        mReport["collation_check_time"] =  ceil(sw.elapsed().count() * 100.0) / 100.0;
    spdlog::info("Parsing complete");
    m_writer->close();

    if (!mTelemetryFile.empty()) {
        parser.report_telemetry(mReport);
    }

    return 0;
}


//  -----------------------------------------------------------------------------
void CFastqParseApp::xSetupOutput()
{
    mpOutStr = &cout;
    if (mOutputFile.empty())
        return;
    mpOutStr = dynamic_cast<ostream*>(new ofstream(mOutputFile, ios::binary));
    mpOutStr->exceptions(std::ofstream::badbit);
}



//  ----------------------------------------------------------------------------
void CFastqParseApp::xBreakdownOutput()
{
    if (mpOutStr != &cout) {
        delete mpOutStr;
    }
}

void CFastqParseApp::xCheckErrorLimits(fastq_error& e )
{
    if (mMaxErrCount == 0)
        throw e;
    auto it = mErrorSet.find(e.error_code());
    if (it == mErrorSet.end())
        throw e;
    spdlog::warn(e.Message());
    if (++mErrorCount >= mMaxErrCount)
        throw fastq_error("Exceeded maximum number of errors {} (code:{})", mMaxErrCount, e.error_code());
}

//  ----------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
    ios_base::sync_with_stdio(false);   // turn off synchronization with standard C streams
    std::locale::global(std::locale("en_US.UTF-8")); // enable comma as thousand separator
    auto stderr_logger = spdlog::stderr_logger_mt("stderr"); // send log to stderr
    spdlog::set_default_logger(stderr_logger);

    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v"); // default logging pattern (datetime, error level, error text)
    return CFastqParseApp().AppMain(argc, argv);
}
