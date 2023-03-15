/* ===========================================================================
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
 * Project:
 *  sratools command line tool
 *
 * Purpose:
 *  provide tool command info
 *
 */

#include <string>
#include <map>
#include <set>
#include <vector>
#include <array>
#include <algorithm>
#include <initializer_list>
#include <iostream>

#include <cstdlib>
#include <cinttypes>

#include "globals.hpp"
#include "debug.hpp"
#include "tool-args.hpp"
#include "command-line.hpp"
#include "build-version.hpp"

#if USE_TOOL_HELP
#define TOOL_HELP(...) {__VA_ARGS__}
#else
#define TOOL_HELP(...) {}
#endif

#define TOOL_ARGS(...) {__VA_ARGS__}

#if USE_TOOL_HELP
#define TOOL_ARG(LONG, ALIAS, ARG, HELP) {HELP, LONG, ALIAS, 0, ARG, false}
#define FQD_TOOL_ARG(LONG, ALIAS, ARG, HELP) {HELP, LONG, ALIAS, 0, (ARG) != 0, (ARG) < 0}
#else
#define TOOL_ARG(LONG, ALIAS, ARG, HELP) {LONG, ALIAS, 0, ARG, false}
#define FQD_TOOL_ARG(LONG, ALIAS, ARG, HELP) {LONG, ALIAS, 0, (ARG) != 0, (ARG) < 0}
#endif

#define TOOL_NAME_COMMON ""
#define TOOL_ARGS_COMMON TOOL_ARGS ( \
    TOOL_ARG("ngc", "", true, TOOL_HELP("PATH to ngc file.", 0)), \
    TOOL_ARG("cart", "", true, TOOL_HELP("To read a kart file.", 0)), \
    TOOL_ARG("perm", "", true, TOOL_HELP("PATH to jwt cart file.", 0)), \
    TOOL_ARG("help", "h?", false, TOOL_HELP("Output a brief explanation for the program.", 0)), \
    TOOL_ARG("version", "V", false, TOOL_HELP("Display the version of the program then quit.", 0)), \
    TOOL_ARG("log-level", "L", true, TOOL_HELP("Logging level as number or enum string.", 0)), \
    TOOL_ARG("verbose", "v", false, TOOL_HELP("Increase the verbosity of the program status messages.", "Use multiple times for more verbosity.", "Negates quiet.", 0)), \
    TOOL_ARG("quiet", "q", false, TOOL_HELP("Turn off all status messages for the program.", "Negates verbose.", 0)), \
    TOOL_ARG("option-file", "", true, TOOL_HELP("Read more options from the file.", 0)), \
    TOOL_ARG("debug", "+", true, TOOL_HELP("Turn on debug output for module.", "All flags if not specified.", 0)), \
    TOOL_ARG("no-user-settings", "", false, TOOL_HELP("Turn off user-specific configuration.", 0)), \
    TOOL_ARG("ncbi_error_report", "", true, TOOL_HELP("Control program execution environment report generation (if implemented).", "One of (never|error|always). Default is error.", 0)), \
    TOOL_ARG(0, 0, 0, TOOL_HELP(0)))

#include "fastq-dump-arguments.h"
#include "tool-arguments.h"

ParameterDefinition const &ParameterDefinition::unknownParameter() {
    static auto const y = ParameterDefinition(TOOL_ARG(nullptr, nullptr, false, {}));
    return y;
}

ParameterDefinition const &ParameterDefinition::argument() {
    static auto const y = ParameterDefinition(TOOL_ARG("", nullptr, true, {}));
    return y;
}

/// Used by `ParamDefinitions` to traverse the command line arguments.
struct ArgvIterator {
private:
    CommandLine const *parent;
    mutable int argind;
    mutable int indarg;
public:
    ArgvIterator(CommandLine const &parent)
    : parent(&parent)
    , argind(0)
    , indarg(0)
    {}

    /// \brief move to next parameter
    ///
    /// In normal mode, moves to next argv element, if any.
    /// In shortArg mode, moves to next short arg, or switches back to normal mode if no more shortArgs.
    ///
    /// \returns 0 if there are no more, 1 if in normal mode, -1 if in shortArg mode.
    int next() const {
        if (argind == parent->argc) return 0;
        if (indarg > 0) {
            ++indarg;
            if (parent->argv[argind][indarg] != '\0')
                return -1;
            indarg = 0;
        }
        ++argind;
        return argind < parent->argc ? 1 : 0;
    }

    /// \brief The current element of argv.
    char const *get() const {
        return parent->argv[argind];
    }

    /// \brief The index in argv of current element of argv.
    int index() const {
        return argind;
    }

    /// \brief Switch to shortArg mode and return the current parameter.
    char const *getChar() const {
        if (indarg == 0)
            indarg = 1;
        return get() + indarg;
    }

    /// \brief End shortArg mode and advance to next parameter.
    void advance() const {
        assert(indarg > 0);
        indarg = 0;
        ++argind;
    }
};

struct CharIndexElement : public std::pair<char, unsigned>
{
    using Base = std::pair<char, unsigned>;

    CharIndexElement(CharIndexElement const &other) = default;
    CharIndexElement(Base const &base)
    : Base(base)
    {};

    bool operator< (char const &query) const {
        return first < query;
    }
    friend bool operator< (char const &a, CharIndexElement const &b) {
        return a < b.first;
    }
};

/// \brief Indexed Argument Definitions for a tool.
struct ParamDefinitions_Common {
    using Container = UniqueOrderedList<ParameterDefinition>;
    using LongIndex = std::map<std::string, Container::Index>;
    using ShortIndex = UniqueOrderedList<CharIndexElement>;

    Container container;
    ShortIndex shortIndex;

    virtual ~ParamDefinitions_Common() {}
    
    ParamDefinitions_Common(size_t count, ParameterDefinition const *defs)
    : container(count)
    {
        for (auto cur = defs; cur != defs + count; ++cur)
            container.insert(*cur);
    }

    ParamDefinitions_Common(ParamDefinitions_Common const &common, size_t count, ParameterDefinition const *defs)
    : container(common.container.size() + count)
    {
        for (auto def : common.container)
            container.insert(def);

        for (auto cur = defs; cur != defs + count; ++cur)
            container.insert(*cur);

        // Update indices and assign bit masks.
        unsigned i = 0;
        uint64_t mask = 1;
        for (auto &def : container) {
            if (def.aliases) {
                for (auto const &ch : std::string(def.aliases))
                    shortIndex.insert(CharIndexElement({ch, i}));
            }
            if (!common.container.contains(def)) {
                def.bitMask = mask;
                mask <<= 1;
            }
            ++i;
        }
    }

    /** Print tool's parameter bits; format is tab-delimited.
     *
     * Fields: tool name, '(' bit shift ')' decimal value, parameter long name
     */
    void printParameterBitmasks(std::string const &tool, std::ostream &out) const {
        char buffer[] = "0x0000000000000001 (1 <<  0)";
        char *numAt = buffer + 17, *const shiftAt = numAt + 9;
        unsigned shift = 0;

        assert(numAt[0] == '1' && numAt[1] == ' ');
        assert(shiftAt[0] = '0' && shiftAt[1] == ')');
        for (auto &def : container) {
            if (def.isArgument()) // obviously, doesn't apply to tool arguments.
                continue;
            if (def.bitMask == 0) // common tool parameters don't have bits assigned to them.
                continue;

            *numAt = "1248"[shift % 4];
            assert(std::stoul(buffer, nullptr, 0) == def.bitMask);

            if (shift > 9)
                shiftAt[-1] = (shift / 10) % 10 + '0';
            shiftAt[0] = shift % 10 + '0';

            out << tool << '\t'
                << sratools::Version::currentString << '\t'
                << buffer << '\t'
                << def.name << '\n';

            ++shift;
            if ((shift % 4) == 0)
                *numAt-- = '0';
            shiftAt[-1] = shiftAt[0] = ' ';
        }
        out << std::flush;
    }

    virtual bool parseArg(Arguments::Container *dst, ArgvIterator const &iter) const = 0;

    Arguments parse(CommandLine const &cmdLine) const {
        Arguments::Container result;
        auto iter = ArgvIterator(cmdLine);

        result.reserve(cmdLine.argc);

        while (parseArg(&result, iter))
            ;

        uint64_t argsHash = 0;

        for (auto const &used : result)
            argsHash |= used.def->bitMask;

        return Arguments(result, argsHash);
    }
};

struct ParamDefinitions final : public ParamDefinitions_Common
{
private:
    ParamDefinitions(size_t count, ParameterDefinition const *defs)
    : ParamDefinitions_Common(count, defs)
    {}
public:
    ParamDefinitions(ParamDefinitions_Common const &common, size_t count, ParameterDefinition const *defs)
    : ParamDefinitions_Common(common, count, defs)
    {}

    /// \brief Find the index of the definition.
    ///
    /// \Returns index of definition and a pointer to parameter's argument if it is attached to the string.
    std::pair< int, char const * > findLong(char const *const arg) const
    {
        auto const eq = strchr(arg, '=');
        auto const fnd = container.find(eq ? std::string(arg, eq - arg) : std::string(arg));

        if (fnd.first != fnd.second)
            return {iterDistance(container.begin(), fnd.first), eq + 1};

        return {-1, nullptr};
    }

    bool parseArg(Arguments::Container *dst, ArgvIterator const &i) const override {
        auto nextIsArg = 0;
        auto index = -1;

        for ( ; ; ) {
            switch (i.next()) {
            case 0:
                if (nextIsArg) {
                    dst->emplace_back(Argument({&container[index], nullptr, -1}));
                    return true;
                }
                return false;
            case 1:
                if (nextIsArg) {
                    dst->emplace_back(Argument({&container[index], i.get(), i.index() - 1}));
                    return true;
                }
                else {
                    auto const arg = i.get();

                    if (arg[0] != '-') {
                        dst->emplace_back(Argument({&ParameterDefinition::argument(), arg, i.index()}));
                        return true;
                    }
                    if (arg[1] == '-') {
                        auto const f = findLong(arg + 2);
                        if (f.first >= 0) {
                            index = f.first;
                            auto const &def = container[index];
                            if (f.second && def.hasArgument) {
                                dst->emplace_back(Argument({&def, f.second, i.index()}));
                                return true;
                            }
                            if (!f.second && !def.hasArgument) {
                                dst->emplace_back(Argument({&def, nullptr, i.index()}));
                                return true;
                            }
                            if (def.hasArgument) {
                                ++nextIsArg;
                                continue;
                            }
                        }
                        dst->emplace_back(Argument({&ParameterDefinition::unknownParameter(), arg, i.index()}));
                        return true;
                    }
                }
                // fallthrough;
            case -1:
                {
                    auto const arg = i.getChar();
                    switch (nextIsArg) {
                    case 0:
                        {
                            auto const f = shortIndex.find(*arg);
                            if (f.first != f.second) {
                                index = f.first->second;
                                auto const &def = container[index];
                                if (def.hasArgument) {
                                    ++nextIsArg;
                                    continue;
                                }
                                dst->emplace_back(Argument({&def, arg, i.index()}));
                                return true;
                            }
                            dst->emplace_back(Argument({&ParameterDefinition::unknownParameter(), arg, i.index()}));
                            return true;
                        }
                    case 1:
                        if (*arg == '=') {
                            ++nextIsArg;
                            continue;
                        }
                        // fallthrough;
                    case 2:
                        dst->emplace_back(Argument({&container[index], arg, i.index()}));
                        i.advance();
                        return true;
                    default:
                        assert(!"reachable");
                    }
                }
                break;
            default:
                assert(!"reachable");
            }
        }
    }

    static inline ParamDefinitions makeCommonParams() {
        static ParameterDefinition const defs[] = TOOL_ARGS_COMMON;
        return ParamDefinitions(sizeof(defs)/sizeof(defs[0]) - 1, defs);
    }
};

struct ParamDefinitions_FQD final : public ParamDefinitions_Common
{
    ParamDefinitions_FQD(ParamDefinitions_Common const &common, size_t count, ParameterDefinition const *defs)
    : ParamDefinitions_Common(common, count, defs)
    {}

    bool parseArg(Arguments::Container *result, ArgvIterator const &iter) const override {
        int index = -1;
        bool nextMayBeArg = false;
        bool nextMustBeArg = false;

        for ( ; ; ) {
            switch (iter.next()) {
            case 0:
                if (nextMayBeArg) {
                    // optional argument did not show up
                    assert(index >= 0 && index < (int)container.size());
                    result->emplace_back(Argument({&container[index], nullptr, iter.index() - 1}));
                }
                return false;
            case 1:
                {
                    auto const arg = iter.get();
                    if (nextMustBeArg || (arg[0] != '-' && nextMayBeArg)) {
                        assert(index >= 0 && index < (int)container.size());
                        result->emplace_back(Argument({&container[index], arg, iter.index() - 1}));
                        return true;
                    }
                    if (arg[0] == '-') {
                        if (nextMayBeArg) {
                            // optional argument did not show up
                            assert(index >= 0 && index < (int)container.size());
                            result->emplace_back(Argument({&container[index], nullptr, iter.index() - 1}));
                        }
                        if (arg[1] == '-') {
                            auto const f = container.find(arg + 2);
                            if (f.first == f.second) {
                                result->emplace_back(Argument({&ParameterDefinition::unknownParameter(), arg, iter.index()}));
                                return true;
                            }
                            index = (int)iterDistance(container.begin(), f.first);
                        }
                        else {
                            auto const f = shortIndex.find(arg[1]);
                            if (f.first == f.second) {
                                result->emplace_back(Argument({&ParameterDefinition::unknownParameter(), arg, iter.index()}));
                                return true;
                            }
                            index = f.first->second;
                        }
                        auto const &def = container[index];
                        if (!def.hasArgument) {
                            result->emplace_back(Argument({&def, nullptr, iter.index()}));
                            return true;
                        }
                        nextMayBeArg = true;
                        nextMustBeArg = !def.argumentIsOptional;
                        continue;
                    }
                    result->emplace_back(Argument({&ParameterDefinition::argument(), arg, iter.index()}));
                    return true;
                }
                break;
            }
        }
        return false;
    }
};

static ParamDefinitions_Common const &commonParams = ParamDefinitions::makeCommonParams();

#define DEFINE_ARGS(NAME, PARSE_TYPE) \
namespace NAME { \
    using Parser = PARSE_TYPE; \
    static auto const toolName = TOOL_NAME_ ## NAME; \
    static ParameterDefinition const defs[] = TOOL_ARGS_ ## NAME; \
    static ParamDefinitions_Common const &parser = Parser(commonParams, sizeof(defs)/sizeof(defs[0]) - 1, defs); \
}

DEFINE_ARGS(FASTERQ_DUMP, ParamDefinitions)
DEFINE_ARGS(SAM_DUMP, ParamDefinitions)
DEFINE_ARGS(VDB_DUMP, ParamDefinitions)
DEFINE_ARGS(SRA_PILEUP, ParamDefinitions)
DEFINE_ARGS(FASTQ_DUMP, ParamDefinitions_FQD)

static ParamDefinitions_Common const &parserForTool(std::string const &toolName)
{
    if (toolName == FASTERQ_DUMP::toolName)
        return FASTERQ_DUMP::parser;

    if (toolName == FASTQ_DUMP::toolName)
        return FASTQ_DUMP::parser;

    if (toolName == VDB_DUMP::toolName)
        return VDB_DUMP::parser;

    if (toolName == SAM_DUMP::toolName)
        return SAM_DUMP::parser;

    if (toolName == SRA_PILEUP::toolName)
        return SRA_PILEUP::parser;

    throw UnknownToolException();
}

Arguments argumentsParsed(CommandLine const &cmdLine)
{
    return parserForTool(cmdLine.toolName).parse(cmdLine);
}

std::ostream &operator <<(std::ostream &out, Argument const &arg) {
    if (arg.isArgument())
        return out << arg.argument;
    if (!arg.def->hasArgument)
        return out << arg.def->name;
    if (!arg.argument)
        return out << arg.def->name << " (null)";
    else
        return out << arg.def->name << " " << arg.argument;
}

void printParameterBitmasks(std::ostream &out) {
    FASTERQ_DUMP::parser.printParameterBitmasks(FASTERQ_DUMP::toolName, out);
    FASTQ_DUMP::parser.printParameterBitmasks(FASTQ_DUMP::toolName, out);
    SAM_DUMP::parser.printParameterBitmasks(SAM_DUMP::toolName, out);
    SRA_PILEUP::parser.printParameterBitmasks(SRA_PILEUP::toolName, out);
    VDB_DUMP::parser.printParameterBitmasks(VDB_DUMP::toolName, out);
}
