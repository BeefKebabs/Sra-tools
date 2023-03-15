#!/bin/bash

bin_dir=$1
srapath=$2


#CONFIGTOUSE=NCBI_SETTINGS Not needed?

echo Testing ${srapath} from ${bin_dir} # CONFIGTOUSE=${CONFIGTOUSE}

# SRP:
# TODO: TEMPORARELY DISABLED:	$(BINDIR)/${srapath} SRP000001 -+VFS # works & does not crash
#${CONFIGTOUSE}=/
output=$(NCBI_SETTINGS=../LIBS-GUID.mkfg ${bin_dir}/${srapath} SRR000001 -fnames > /dev/null)

res=$?
if [ "$res" != "0" ];
	then echo "SRP test FAILED, res=$res output=$output" && exit 1;
fi

echo SRP test is finished

# vdbcache:
output=$(NCBI_SETTINGS=../LIBS-GUID.mkfg ${bin_dir}/${srapath} SRR1557953 | wc -l | perl check-cnt.pl)

res=$?
if [ "$res" != "0" ];
	then echo "vdbcache test FAILED, res=$res output=$output" && exit 1;
fi

echo vdbcache test is finished