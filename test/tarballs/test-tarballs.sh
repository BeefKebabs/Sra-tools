#!/bin/bash
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

echo $0 $*

#
#  Download and test SRA Toolkit tarballs (see VDB-1345)
#  Errors are reported to the specified email
#
# Parameters: $1 - working dir (will contain a copy of the latest md5sum.txt file) 
#             $2 - version of sra-toolkit, e.g. 2.10.7 ("current by default)
#             $3 - version of ngs, e.g. 2.10.7 ("current by default)
#             $4 - 'nojava' to skip anything that needs java
#
# return codes:
# 0 - tests passed
# 1 - wget sratoolkit failed
# 2 - gunzip sratoolkit failed
# 3 - tar sratoolkit failed
# 4 - wget GenomeAnalysisTK.jar failed
# 5 - wget ngs-sdk failed
# 6 - gunzip ngs-sdk failed
# 7 - tar ngs-sdk failed
# 8 - one of smoke tests failed
# 9 - example failed

DEF_WORKDIR="./temp"
WORKDIR="${1:-$DEF_WORKDIR}"

DEF_VERS="current"
VERS="${2:-$DEF_VERS}"
NGSVERS="${3:-$DEF_VERS}"

echo "Testing sra-tools tarballs, working directory = $WORKDIR"


case $(uname) in
Linux)
    python -mplatform | grep -q Ubuntu && OS=ubuntu64 || OS=centos_linux64
    TOOLS="${TOOLS} pacbio-load remote-fuser"
    realpath() {
        readlink -f $1
    }
    get() {
        echo wget "$1"
        wget -q --no-check-certificate "$1"
    }
    uname=linux
    ;;
Darwin)
    OS=mac64
    realpath() {
        [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
    }
    get() {
        echo curl "$1"
        curl --fail --silent --insecure --remote-name "$1"
    }
    uname=mac
    ;;
esac
HOMEDIR=$(dirname $(realpath $0))

################################## sratoolkit ##################################

SDK_URL="https://ftp-trace.ncbi.nlm.nih.gov/sra/sdk/${VERS}/"
TK_TARGET="sratoolkit.${VERS}-${OS}"

rm -rv ${WORKDIR}
mkdir -p ${WORKDIR}
OLDDIR=$(pwd)
cd ${WORKDIR}

df -h .
get "${SDK_URL}${TK_TARGET}.tar.gz" || exit 1
gunzip -f "${TK_TARGET}.tar.gz" || exit 2
TK_PACKAGE=$(tar tf "${TK_TARGET}.tar" | head -n 1)
rm -rf "${TK_PACKAGE}"
tar xf "${TK_TARGET}.tar" || exit 3

# extract version number from the package's name
[[ "${TK_PACKAGE}" =~ \.[0-9]+\.[0-9]+\.[0-9]+ ]] && VERSION="${BASH_REMATCH[0]:1}" # clip leading '.'
echo Current version: "${VERSION}"

################################## smoke-test ##################################

VDB_CONFIG_FILE=`pwd`/${TK_PACKAGE}bin/ncbi
echo $HOMEDIR/smoke-test.sh ./${TK_PACKAGE} ${VERSION}
#ls `pwd`/${TK_PACKAGE}/bin/ncbi
     $HOMEDIR/smoke-test.sh ./${TK_PACKAGE} ${VERSION}
RC=$?

if [ "${RC}" != "0" ]
then
    echo "Smoke test returned ${RC}"
    exit 8
fi

if [ "$4" = "yesjava" ]
then

############################### GenomeAnalysisTK ###############################

    GATK_TARGET=GenomeAnalysisTK.jar
    get "${SDK_URL}${GATK_TARGET}" || exit 4

################################### ngs-sdk ####################################

    NGS_URL="https://ftp-trace.ncbi.nlm.nih.gov/sra/ngs/${NGSVERS}/"
    NGS_TARGET="ngs-sdk.${NGSVERS}-${uname}"
    get "${NGS_URL}${NGS_TARGET}.tar.gz" || exit 5
    gunzip -f "${NGS_TARGET}.tar.gz" || exit 6
    NGS_PACKAGE=$(tar tf "${NGS_TARGET}.tar" | head -n 1)
    rm -rf "${NGS_PACKAGE}"
    tar xf "${NGS_TARGET}.tar" || exit 7

################################## smoke-test ##################################

    echo "/LIBS/GUID = \"8badf00d-1111-4444-8888-deaddeadbeef\"" > ./${TK_PACKAGE}bin/ncbi/local.kfg
    echo VDB_CONFIG=`pwd`/${TK_PACKAGE}bin/ncbi $HOMEDIR/smoke-test-ngs.sh ./${TK_PACKAGE} ${VERSION}
    #ls `pwd`/${TK_PACKAGE}bin/ncbi
         VDB_CONFIG=`pwd`/${TK_PACKAGE}bin/ncbi $HOMEDIR/smoke-test-ngs.sh ./${TK_PACKAGE} ${VERSION}
    RC=$?

    if [ "${RC}" != "0" ]
    then
        echo "Smoke test returned ${RC}"
        exit 8
    fi

    echo rm ${GATK_TARGET} ${NGS_PACKAGE} ${NGS_TARGET}.tar
    rm -rf  ${GATK_TARGET} ${NGS_PACKAGE} ${NGS_TARGET}.tar *vcf*
fi

echo rm ${TK_PACKAGE} ${TK_TARGET}.tar
rm -rf  ${TK_PACKAGE} ${TK_TARGET}.tar
cd ${OLDDIR} && ( rm -rf ${WORKDIR} || ls ${WORKDIR} )
