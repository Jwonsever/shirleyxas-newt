#!/bin/bash -l
#moves .cube file to server from hopper

dir=$1
molName=$2
state=$3

file="${dir}state.${state}.cube"
tmpDir="/project/projectdirs/als/www/james-xs/Shirley-data/tmp/${molName}"

rm -rf $tmpDir
mkdir $tmpDir
cp ${file} "${tmpDir}/"
if [ $? -ne 0 ]; then
echo "File does not exist"
fi

chmod 777 $tmpDir
cd $tmpDir
chmod 777 *
wait

#find some way to empty the directories
