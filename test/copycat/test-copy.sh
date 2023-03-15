#!/bin/bash

bin_dir=$1
tool_binary=$2
FILE="1.xml"

echo "testing ${tool_binary} copy"
mkdir -p actual
rm -rf actual/${FILE}

output=$(${bin_dir}/${tool_binary} input/${FILE} actual/ && diff -q input/${FILE} actual/${FILE})

res=$?
if [ "$res" != "0" ];
	then echo "${tool_binary} copy failed, res=${res} output=${output}" && exit 1;
fi

echo "${tool_binary} copy succeeded"
