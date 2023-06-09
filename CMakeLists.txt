# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================

cmake_minimum_required (VERSION 3.16)
if (WIN32)
    cmake_minimum_required (VERSION 3.20)
	set (CMAKE_SYSTEM_VERSION 10.0 CACHE STRING INTERNAL FORCE)
endif()

# set( CMAKE_DISABLE_SOURCE_CHANGES ON ) # Cannot use with bison
# set( CMAKE_DISABLE_IN_SOURCE_BUILD ON )

option(TOOLS_ONLY "Generate targets for tools only (else tools and tests)" ON)
option(BUILD_TOOLS_LOADERS "Generate targets for loader tools")
option(BUILD_TOOLS_INTERNAL "Generate targets for internal tools")
option(BUILD_TOOLS_TEST_TOOLS "Generate targets for test tools")
option(RUN_SANITIZER_TESTS "Run ASAN and TSAN tests" OFF)
option(NO_JAVA "Do not generate any targets that need Java" OFF)

message(CMAKE_VERSION = ${CMAKE_VERSION})

#set( CMAKE_VERBOSE_MAKEFILE ON )

# best to use at the top
project ( sra-tools )
if( PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR )
    message(FATAL_ERROR "In-source builds are not allowed")
endif()

# this has to follow the project() command!
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_CURRENT_SOURCE_DIR}/build"
)

set( VDB_BINDIR "${CMAKE_BINARY_DIR}/../ncbi-vdb" CACHE PATH "Location of NCBI VDB build directory" )
set( VDB_INCDIR "${CMAKE_SOURCE_DIR}/../ncbi-vdb/interfaces" CACHE PATH "Location of NCBI VDB include directory" )

include(CTest)
enable_testing()
include( build/env.cmake NO_POLICY_SCOPE )

add_subdirectory( shared )
add_subdirectory( ngs )
add_subdirectory( libs )
add_subdirectory( tools )

if ( NOT TOOLS_ONLY )
    add_subdirectory( test )
endif()

set ( CPACK_PACKAGE_NAME sra-tools )
set ( CPACK_PACKAGE_VERSION 0.1 )
set ( CPACK_PACKAGE_CONTACT "sra-tools@ncbi.nlm.nih.gov" )
set ( CPACK_PACKAGE_DESCRIPTION_SUMMARY "tax payers funded SRA toolkit for scientists" )

set ( CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64 )

include ( CPack )
