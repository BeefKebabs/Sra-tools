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
 *  Provide file system path support.
 *
 */

#if WINDOWS

#include "util.hpp"
#include "util.win32.hpp"
#include "file-path.hpp"
#include <ios>
#include <new>

#define NUL_W (L'\0')
#define DOT_W (L'.')
#define SEP_W (L'\\')
#define SEP_POSIX_W (L'/')

#define CONST_CAST_W(X) (reinterpret_cast<wchar_t const *>(X))
#define CAST_W(X) (reinterpret_cast<wchar_t *>(X))

#define NUL_N ('\0')
#define DOT_N ('.')
#define SEP_N ('\\')
#define SEP_POSIX ('/')

#define CONST_CAST_N(X) (reinterpret_cast<char const *>(X))
#define CAST_N(X) (reinterpret_cast<char *>(X))

#if USE_WIDE_API
using API_CHAR = wchar_t;
#define API_IS_WIDE (true)
#define API_NUL NUL_W
#define API_SEP SEP_W
#define API_DOT DOT_W
#define SELF_DIR L"."
#define API_SEP_POSIX (L'/')
#else
using API_CHAR = char;
#define API_IS_WIDE (false)
#define API_NUL NUL_N
#define API_SEP SEP_N
#define API_DOT DOT_N
#define SELF_DIR "."
#define API_SEP_POSIX ('/')
#endif
using API_CONST_STRING = API_CHAR const *;
using API_STRING = API_CHAR *;

#define API_CONST_CAST(X) (reinterpret_cast<API_CONST_STRING>(X))
#define API_CAST(X) (reinterpret_cast<API_STRING>(X))
#define API_CONST_PATH API_CONST_CAST(this->path)
#define API_PATH API_CAST(this->path)
#define WIDE_CONST_PATH CONST_CAST_W(this->path)
#define NARROW_CONST_PATH CONST_CAST_N(this->path)
#define WIDE_PATH CAST_W(this->path)
#define NARROW_PATH CAST_N(this->path)

#define WIDE(F) wide().F
#define NARROW(F) narrow().F
#define COPY(F) copy().F

static void throw_system_error [[noreturn]] (HRESULT hr, char const *function) {
    throw std::system_error(std::error_code(int(hr & 0xFFFF), std::system_category()), function);
}


template < typename T >
static Win32Support::auto_free_ptr< T > newAutoFree(size_t count)
{
    auto const tmp = (T *)malloc(sizeof(T) * count);
    if (tmp)
        return Win32Support::auto_free_ptr< T >(tmp);
    throw std::bad_alloc();
}

static inline size_t measure(char const *const string)
{
    return strlen(string);
}

static inline size_t measure(wchar_t const *const string)
{
    return wcslen(string);
}

static inline size_t measure(void const *const string, bool isWide)
{
    return isWide ? measure(CONST_CAST_W(string)) : measure(CONST_CAST_N(string));
}

static inline bool isDirSeperator(char const ch)
{
    return ch == '\\' || ch == '/';
}

static inline bool isDirSeperator(wchar_t const ch)
{
    return ch == L'\\' || ch == L'/';
}

static inline bool isPathSeperator(API_CHAR const ch)
{
#if USE_WIDE_API
    return isDirSeperator(ch) || ch == L':';
#else
    return isDirSeperator(ch) || ch == ':';
#endif
}

static inline size_t getFullPathName(void const *path, size_t length = 0, void *result = nullptr) {
#if USE_WIDE_API
 #define FUNC GetFullPathNameW
#else
 #define FUNC GetFullPathNameA
#endif
    API_CHAR dummy;
    auto const len = FUNC(reinterpret_cast<API_CONST_STRING>(path)
                          , length
                          , length == 0 ? &dummy : reinterpret_cast<API_STRING>(result)
                          , nullptr);
    if (len == 0)
        throw_system_error("GetFullPathName");
    return len;
#undef FUNC
}

static inline bool pathFileExists(void const *path)
{
#if USE_WIDE_API
 #define FUNC PathFileExistsW
#else
 #define FUNC PathFileExistsA
#endif
    return FALSE != FUNC(reinterpret_cast<API_CONST_STRING>(path));
#undef FUNC
}

static inline bool pathDirExists(void const *path)
{
#if USE_WIDE_API
 #define FUNC PathIsDirectoryW
#else
 #define FUNC PathIsDirectoryA
#endif
    return FALSE != FUNC(reinterpret_cast<API_CONST_STRING>(path));
#undef FUNC
}

static inline HANDLE createFile(void const *path, int mode, int share, int createMode, int createFlags)
{
#if USE_WIDE_API
 #define FUNC CreateFileW
#else
 #define FUNC CreateFileA
#endif
    return FUNC(reinterpret_cast<API_CONST_STRING>(path)
                 , mode
                 , share
                 , NULL
                 , createMode
                 , createFlags
                 , NULL);
#undef FUNC
}

static inline bool isExecutable(void const *path)
{
#if USE_WIDE_API
 #define FUNC GetBinaryTypeW
#else
 #define FUNC GetBinaryTypeA
#endif
    DWORD dummy = 0;
    return FUNC(reinterpret_cast<API_CONST_STRING>(path), &dummy) == 0 ? false : true;
#undef FUNC
}

static inline void setCurrentDirectory(API_CONST_STRING path)
{
#if USE_WIDE_API
 #define FUNC SetCurrentDirectoryW
#else
 #define FUNC SetCurrentDirectoryA
#endif
    if (FUNC(path) != TRUE)
        throw_system_error("SetCurrentDirectory");
#undef FUNC
}

static inline size_t getCurrentDirectory(API_STRING buffer, size_t maxlen)
{
#if USE_WIDE_API
 #define FUNC GetCurrentDirectoryW
#else
 #define FUNC GetCurrentDirectoryA
#endif
    return (size_t)FUNC(maxlen, buffer);
#undef FUNC
}

static inline size_t getModuleFileName(API_STRING buffer, size_t size)
{
#if USE_WIDE_API
 #define FUNC GetModuleFileNameW
#else
 #define FUNC GetModuleFileNameA
#endif
    return FUNC(NULL, buffer, size);
#undef FUNC
}

static inline API_CONST_STRING pathFindNextComponent(API_CONST_STRING path)
{
#if USE_WIDE_API
    return PathFindNextComponentW(path);
#else
    return PathFindNextComponentA(path);
#endif
}

/// Does path start at the root of some volume?
static inline bool pathIsRoot(API_CONST_STRING path)
{
#if USE_WIDE_API
    return PathIsRootW(path);
#else
    return PathIsRootA(path);
#endif
}

/// Convert POSIX path seperators to Windows ones.
/// In-place version.
static void pathCleanUpSeperators(wchar_t *path)
{
    for ( ; ; ++path) {
        auto const ch = *path;
        if (ch == L'\0')
            return;
        if (ch == SEP_POSIX_W)
            *path = SEP_W;
    }
}

/// Convert POSIX path seperators to Windows ones.
/// Copy version.
static void pathCleanUpSeperators(wchar_t *path, wchar_t const *inpath)
{
    for ( ; ; ) {
        auto const ch = *inpath++;
        *path++ = (ch == SEP_POSIX_W) ? SEP_W : ch;
        if (ch == L'\0')
            return;
    }
}

/// Canonicalized a path. There is no narrow version of this call.
///
/// NB. Must canonicalize the path seperators **first**!
///
/// The returned pointer needs to be free'd with `LocalFree`.
static inline wchar_t *pathCanonicalize(wchar_t const *path)
{
    wchar_t *result = nullptr;
    auto const rc = PathAllocCanonicalize(path, PATHCCH_FORCE_ENABLE_LONG_NAME_PROCESS|PATHCCH_ALLOW_LONG_PATHS, &result);
    if (rc != S_OK)
        throw_system_error(rc
                           , "PathAllocCanonicalize");
    return result;
}

/// Canonicalize a path (and path seperators).
///
/// The returned pointer needs to be free'd with `LocalFree`.
static wchar_t *canonicalPathW(NativeString const &path)
{
    // must make an editable copy in order to clean up path seperators
#if USE_WIDE_API
    auto dup = std::vector<wchar_t>(path.size() + 1, L'\0');
    auto const dupp = &dup[0];
    pathCleanUpSeperators(dupp, path.c_str());
#else
    auto dup = Win32Support::makeWide(path.c_str());
    auto const dupp = dup.get();
    pathCleanUpSeperators(dupp);
#endif
    
    return pathCanonicalize(dupp);
}

/// The canonicalized path, in the same representation as the input path.
static NativeString canonicalPath(NativeString const &path)
{
    auto const cpath = canonicalPathW(path);
    auto const &result = NativeString(
#if USE_WIDE_API
                                      cpath
#else
                                      Win32Support::makeUnwide(cpath).get()
#endif
                                      );
    LocalFree(cpath);
    return result;
}

/// Combine two paths. There is no narrow version of this call.
///
/// The returned pointer needs to be free'd with `LocalFree`.
static inline wchar_t *pathCombineW(wchar_t *left, wchar_t *right)
{
    wchar_t *result = nullptr;
    auto const rc = PathAllocCombine(left, right, PATHCCH_FORCE_ENABLE_LONG_NAME_PROCESS|PATHCCH_ALLOW_LONG_PATHS, &result);
    LocalFree(right);
    LocalFree(left);
    if (rc != S_OK)
        throw_system_error(rc
                           , "PathAllocCombine");
    return result;
}

static NativeString pathCombine(NativeString const &left, NativeString const &right)
{
    auto const wresult = pathCombineW(canonicalPathW(left), canonicalPathW(right));
    auto const &result = NativeString(
#if USE_WIDE_API
                                      wresult
#else
                                      Win32Support::makeUnwide(wresult.c_str()).get()
#endif
                                      );
    LocalFree(wresult);

    return result;
}

FilePath::operator std::string() const
{
#if USE_WIDE_API
    return std::string(Win32Support::makeUnwide(path.c_str()).get());
#else
    return path;
#endif
}

#if USE_WIDE_API
FilePath::operator std::wstring() const
{
#if USE_WIDE_API
    return path;
#else
    return std::wstring(Win32Support::makeWide(path.c_str()).get());
#endif
}
#endif

static NativeString trimPath(NativeString const &path, bool canTrim = true)
{
    auto const trimLen = (canTrim && path.size() > 1 && path.back() == API_SEP) ? 1 : 0;
    return path.substr(0, trimLen);
}

/// Locate all the parts of a path, as defined by Windows.
static std::vector< NativeString::value_type const * > pathParts(NativeString const &path)
{
    auto result = std::vector< NativeString::value_type const * >();
    if (path.empty())
        return result;
    for (auto p = path.c_str(); p && *p; p = pathFindNextComponent(p))
        result.push_back(p);
    /*
     c:\foo\ -> [ c:\foo\ foo\ ]
     */
    return result;
}

std::pair< FilePath, FilePath > FilePath::split() const
{
    auto const cpath = canonicalPath(path);
    auto const isRoot = pathIsRoot(cpath.c_str());
    auto const &parts = pathParts(cpath);
    
    switch (parts.size()) {
    case 0:
        return std::make_pair(FilePath(), FilePath());
    case 1:
        if (isRoot)
            return std::make_pair(FilePath(cpath), FilePath());
        else
            return std::make_pair(FilePath(), FilePath(cpath));
    default:
        break;
    }
    auto const last = parts.front() - parts.back();
    auto const &basename = trimPath(cpath.substr(last));
    auto const &dirname = trimPath(cpath.substr(0, last), !isRoot || parts.size() > 2);
    
    return std::make_pair(FilePath(dirname), FilePath(basename));
}

bool FilePath::exists(std::string const &path)
{
#if USE_WIDE_API
    auto const wstr = Win32Support::makeWide(path.c_str());
    return pathFileExists(wstr.get()) || pathDirExists(wstr.get());
#else
    return pathFileExists(path.c_str()) || pathDirExists(path.c_str());
#endif
}

bool FilePath::exists() const
{
    return pathFileExists(path.c_str()) || pathDirExists(path.c_str());
}

bool FilePath::executable() const
{
    return isExecutable(path.c_str());
}

bool FilePath::readable() const
{
    DWORD flagsAndAttributes = 0;
    
    if (pathFileExists(path.c_str()))
        flagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
    else if (pathDirExists(path.c_str()))
        flagsAndAttributes = FILE_FLAG_BACKUP_SEMANTICS;
    else
        return false;

    auto const fh = createFile(path.c_str(), GENERIC_READ                               // open mode
                               , FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE // sharing mode
                               , OPEN_EXISTING                                          // create mode here means "don't create"
                               , flagsAndAttributes);                                   // create flags
    if (fh != INVALID_HANDLE_VALUE) {
        CloseHandle(fh);
        return true;
    }
    return false;
}

FilePath FilePath::append(FilePath const &element) const
{
    return FilePath(pathCombine(path, element.path));
}

FilePath::FilePath(NativeString const &in)
: path(in)
{}

#if USE_WIDE_API
FilePath::FilePath(std::string const &in)
: path(NativeString(Win32Support::makeWide(in.c_str()).get()))
{}
#endif

bool FilePath::removeSuffix(size_t const count)
{
    if (count > 0 && path.size() >= count) {
        path.resize(path.size() - count);
        return true;
    }
    return count == 0;
}

FilePath FilePath::cwd()
{
    auto const need = getCurrentDirectory(nullptr, 0) + 1;
    auto result = NativeString(need, 0);
    auto const got = getCurrentDirectory(&result[0], need);
    return FilePath(result.substr(0, got));
}

void FilePath::changeDirectory() const
{
    if (!path.empty()) {
        auto temp = path + NativeString(1, API_SEP);
        setCurrentDirectory(temp.c_str());
    }
}

static FilePath fullPathToExe()
{
    NativeString::value_type sbuf[4096];
    {
        auto const sz = getModuleFileName(sbuf, sizeof(sbuf));
        if (sz < sizeof(sbuf))
            return FilePath(NativeString(sbuf, sbuf + sz));
    }
    for (auto result = NativeString(2 * sizeof(sbuf), 0); ; result.resize(result.size() * 2, 0))
    {
        auto const sz = getModuleFileName(&result[0], result.size());
        if (sz < result.size())
            return FilePath(result.substr(0, sz));
    }
}

FilePath FilePath::fullPathToExecutable(char const *const *const argv, char const *const *const envp, char const *const *const extra)
{
    return fullPathToExe();
    (void)argv; (void)envp; (void)extra;
}

#if USE_WIDE_API
FilePath FilePath::fullPathToExecutable(wchar_t const *const *const argv, wchar_t const *const *const envp, char const *const *const extra)
{
    return fullPathToExe();
    (void)argv; (void)envp; (void)extra;
}
#endif

#else
#endif
