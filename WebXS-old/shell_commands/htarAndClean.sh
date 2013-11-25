#!/bin/bash

# Load Global Variables
scriptDir=`dirname $0`
. $scriptDir/../../GlobalValues.in

if [ $# -ne 1 ]
then
    echo "Arguments are not valid."
    echo "Usage: "`echo $0`" directory"
    exit
fi


#$1 is the directory to HTAR
echo $1
jobname=`basename $1`

#tar up everything
/usr/common/mss/bin/htar -cf $jobname.tar $1

#Only leave the necessary files for analysis and viewing
$scriptDir/wipeExcessFiles.sh $1