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
 *  parse command line arguments
 *
 */

#pragma once

#include "tool-args.hpp"
#include "file-path.hpp"

struct CommandLine {
    /// the pointer is saved here; its contents could be modified elsewhere. For Windows wchar_t API, it is derived from wargv.
    char const *const *argv;

    /// the pointer is saved here; its contents could be modified elsewhere. For Windows wchar_t API, it is derived from wenvp.
    char const *const *envp;

    /// the pointer is saved here; its contents could be modified elsewhere. For Windows, it is NULL.
    char const *const *extra;

#if USE_WIDE_API
    wchar_t const* const* wargv;
    wchar_t const* const* wenvp;
#endif

#if WINDOWS
#else
    ///< from environment variable (usually NULL)
    char const *fakeName;
#endif

    ///< the name of the executable from argv0, with any path removed.
    char const *baseName;

    ///< the short, logical name of the tool.
    std::string toolName;

    ///< the full path to the executable.
    FilePath fullPathToExe;

    ///< the full path to directory containing the executable.
    FilePath fullPath;

    FilePath toolPath;

    ///< the actual name of the executable, i.e. sratools.3.0.1
    std::string realName;

    int argc;

    uint32_t buildVersion;
#if WINDOWS
#else
    uint32_t versionFromName, runAsVersion;
#endif

    FilePath pathForArgument(int index) const {
#if USE_WIDE_API
        return FilePath(wargv[index]);
#else
        return FilePath(argv[index]);
#endif
    }
    FilePath pathForArgument(Argument const &arg) const {
        return pathForArgument(arg.argind);
    }

    /// Used by normal main
    CommandLine(int argc, char *argv[], char *envp[], char *extra[]);

#if USE_WIDE_API
    /// Used by wmain
    /// Internally, the program is UTF-8 encoded Unicode.
    /// The original command line is saved though,
    /// and reused for launching the child process.
    CommandLine(int argc, wchar_t *wargv[], wchar_t *wenvp[], char *extra[]);
#endif

    ~CommandLine();

#if WINDOWS
    // fakeName not used on Windows
#if USE_WIDE_API
    wchar_t const *runAs() const {
        return wargv[0];
    }
#else
    char const *runAs() const {
        return argv[0];
    }
#endif
#else
    char const *runAs() const {
        return fakeName ? fakeName : argv[0];
    }
#endif

private:
#if WINDOWS
#else
    std::string versionString() const;
#endif

    FilePath getToolPath() const;

#if WINDOWS
#else
    char const *getFakeName() const;
#endif
};
