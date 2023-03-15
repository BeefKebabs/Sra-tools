#!/bin/bash

bin_dir=$1
vdb_validate=$2

echo "testing ${vdb_validate} ${bin_dir}"

TEMPDIR=.

rm -rf actual/
mkdir -p actual/

	output=$(./runtestcase.sh \
	       "${bin_dir}/${vdb_validate} db/sdc_len_mismatch.csra" no_sdc_checks 0)
	res=$?
	if [ "$res" != "0" ];
		then echo "${vdb_validate} FAILED, res=$res output=$output" && exit 1;
	fi

	output=$(./runtestcase.sh \
	       "${bin_dir}/${vdb_validate} db/sdc_tmp_mismatch.csra --sdc:rows 100%" \
	                                  sdc_tmp_mismatch 3)
	res=$?
	if [ "$res" != "0" ];
		then echo "${vdb_validate} FAILED, res=$res output=$output" && exit 1;
	fi
	output=$(./runtestcase.sh \
	       "${bin_dir}/${vdb_validate} db/sdc_pa_longer.csra --sdc:rows 100%" \
	                                  sdc_pa_longer_1 3)
	res=$?
	if [ "$res" != "0" ];
		then echo "${vdb_validate} FAILED, res=$res output=$output" && exit 1;
	fi
	output=$(./runtestcase.sh "${bin_dir}/${vdb_validate} db/sdc_pa_longer.csra --sdc:rows 100% --sdc:plen_thold 50%" sdc_pa_longer_2 3)
	res=$?
	if [ "$res" != "0" ];
		then echo "${vdb_validate} FAILED, res=$res output=$output" && exit 1;
	fi
	output=$(./runtestcase.sh "${bin_dir}/${vdb_validate} db/sdc_pa_longer.csra --sdc:rows 100% --sdc:plen_thold 51%" sdc_pa_longer_3 0)
	res=$?
	if [ "$res" != "0" ];
		then echo "${vdb_validate} FAILED, res=$res output=$output" && exit 1;
	fi
	output=$(./runtestcase.sh "${bin_dir}/${vdb_validate} db/sdc_len_mismatch.csra --sdc:rows 100% --sdc:plen_thold 1%" sdc_len_mismatch_1 3)
	res=$?
	if [ "$res" != "0" ];
		then echo "${vdb_validate} FAILED, res=$res output=$output" && exit 1;
	fi
	output=$(./runtestcase.sh "${bin_dir}/${vdb_validate} db/sdc_len_mismatch.csra --sdc:rows 100% --sdc:plen_thold 100%" sdc_len_mismatch_2 3)
	res=$?
	if [ "$res" != "0" ];
		then echo "${vdb_validate} FAILED, res=$res output=$output" && exit 1;
	fi
	output=$(./runtestcase.sh "${bin_dir}/${vdb_validate} db/sdc_seq_cmp_read_len_corrupt.csra --sdc:seq-rows 100%" sdc_seq_cmp_read_len_corrupt 3)
	res=$?
	if [ "$res" != "0" ];
		then echo "${vdb_validate} FAILED, res=$res output=$output" && exit 1;
	fi
	output=$(./runtestcase.sh "${bin_dir}/${vdb_validate} db/sdc_seq_cmp_read_len_fixed.csra --sdc:seq-rows 100%" sdc_seq_cmp_read_len_fixed 0)
	res=$?
	if [ "$res" != "0" ];
		then echo "${vdb_validate} FAILED, res=$res output=$output" && exit 1;
	fi
	output=$(./runtestcase.sh "${bin_dir}/${vdb_validate} db/blob-row-gap.kar" ROW_GAP 0
	./runtestcase.sh "${bin_dir}/${vdb_validate} db/SRR053990 -Cyes" \
	                   CONSISTENCY 0)
	res=$?
	if [ "$res" != "0" ];
		then echo "${vdb_validate} FAILED, res=$res output=$output" && exit 1;
	fi

	if [ "${TEST_DATA}" != "" ]; then ./runtestcase.sh \
	    "${bin_dir}/${vdb_validate} \
	                ${TEST_DATA}/SRR1207586-READ_LEN-vs-READ-mismatch \
	                -Cyes" READ_LEN 3 ; fi

	# verify failure verifying ancient no-schema run
	if ${bin_dir}/${vdb_validate} db/SRR053325-no-schema 2> actual/noschema; \
	 then echo ${vdb_validate} no-schema-run should fail; exit 1; fi
	grep -q ' Run File is obsolete. Please download the latest' actual/noschema

	echo "All ${vdb_validate} tests succeed"
	rm -rf actual/
