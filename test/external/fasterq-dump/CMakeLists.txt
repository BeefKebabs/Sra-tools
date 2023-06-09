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

if ( NOT WIN32 )
if ( EXISTS "${DIRTOTEST}/fasterq-dump${EXE}" )

    add_test( NAME Test_FasterqDump_Help
        COMMAND ${BINDIR}/fasterq-dump -h
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

    add_test( NAME Test_FasterqDump_NotZeroWithoutParameters
        COMMAND ${BINDIR}/fasterq-dump
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
    set_tests_properties(Test_FasterqDump_NotZeroWithoutParameters PROPERTIES WILL_FAIL TRUE)

    if( RUN_SANITIZER_TESTS )
        add_test( NAME Test_FasterqDump_Help-asan
            COMMAND ${BINDIR}/fasterq-dump-asan -h
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

        add_test( NAME Test_FasterqDump_NotZeroWithoutParameters-asan
            COMMAND ${BINDIR}/fasterq-dump-asan
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
        set_tests_properties(Test_FasterqDump_NotZeroWithoutParameters-asan PROPERTIES WILL_FAIL TRUE)

        add_test( NAME Test_FasterqDump_Help-tsan
            COMMAND ${BINDIR}/fasterq-dump-tsan -h
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

        add_test( NAME Test_FasterqDump_NotZeroWithoutParameters-tsan
            COMMAND ${BINDIR}/fasterq-dump-tsan
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
        set_tests_properties(Test_FasterqDump_NotZeroWithoutParameters-tsan PROPERTIES WILL_FAIL TRUE)
    endif()

else()
	message(WARNING "${DIRTOTEST}/fasterq-dump${EXE} is not found. The corresponding tests are skipped." )
endif()
else()
#TODO: make run on Windows
endif()
