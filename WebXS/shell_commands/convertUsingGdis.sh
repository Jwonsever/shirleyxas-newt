#!/bin/bash

# Load Global Variables
scriptDir=`dirname $0`
. $scriptDir/../../GlobalValues.in

homedir=$CODE_BASE_DIR/$TEMPORARY_FILES;

cd $homedir;
echo `pwd`
infile=${1}
outfile=${2}

echo "copy ${infile} ${outfile}" | ../../WebXS/gdis-0.90/gdis

#Run Cleanup
rm $infile
chmod 777 $infile$outfile