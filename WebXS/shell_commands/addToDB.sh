#!/bin/bash -l
#Copies spectra files into database

dir=$1
molName=$2
date=$3
elem=$4
XCH=$5
user=$6
basedir=$7

# Load Global Variables
scriptDir=`dirname $0`
. $scriptDir/../../GlobalValues.in

#Note, this is tool location dependant, not universal!  todo.
database=$CODE_BASE_DIR/$DATA_LOC/spectraDB/${molName}${date}

mkdir $database
cd $database

cp -r $dir ./Spectrum-${elem}

cd $basedir
cp -r *.xyz $database
cd $database

inputs="${molName:0:23}\t"
if [ ${#molName} -lt 16 ]
then
    inputs="${inputs}\t"
    if [ ${#molName} -lt 8 ]
    then
	inputs="${inputs}\t"
    fi
fi

inputs="${inputs}${date}\t${user:0:15}\t"
if [ ${#user} < 8 ]
then
    inputs="${inputs}\t"
fi

inputs="${inputs}${elem}\t${XCH}\t${molName}${date}/Spectrum-${elem}"

chmod 777 *
cd ..
chmod 777 "${molName}${date}"

echo -e ${inputs} >> index.dat
