#!/bin/bash

# Load Global Variables
scriptDir=`dirname $0`
. $scriptDir/../../GlobalValues.in

homedir=$CODE_BASE_DIR/$TEMPORARY_FILES;

cd $homedir;
echo `pwd`
infile=${1}
cifParams=${2}

echo "copy ${infile} .cif" | ../../gdis-0.90/gdis
echo -e ${cifParams} >> ${infile}.cif
echo "copy ${infile}.cif .xyz" | ../../gdis-0.90/gdis

#Run Cleanup
rm $infile
rm $infile.cif
mv $infile.cif.xyz $infile

chmod 777 $infile*