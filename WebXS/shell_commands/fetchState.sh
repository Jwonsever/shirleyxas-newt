#!/bin/bash -l
#moves .cube file to server from hopper

# Load Global Variables
scriptDir=`dirname $0`
. $scriptDir/../../GlobalValues.in


dir=$1
molName=$2
state=$3

file="${dir}state.${state}.cube.bz2"
tmpDir="${CODE_BASE_DIR}/${TEMPORARY_FILES}/${molName}"

rm -rf $tmpDir
mkdir $tmpDir
cp ${file} "${tmpDir}/"
if [ $? -ne 0 ]; then
    bzip2 -k9 ${dir}state.${state}.cube
    cp ${file} "${tmpDir}/"
    if [ $? -ne 0 ]; then
	echo "File does not exist"
    else
	cp ${file} "${tmpDir}/"
    fi
fi

chmod 777 $tmpDir
cd $tmpDir
chmod 777 *

#unzip
bzip2 -d ./state.${state}.cube.bz2

wait