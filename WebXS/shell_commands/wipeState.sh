#!/bin/bash -l
#deletes .cube files

molName=$1

# Load Global Variables
scriptDir=`dirname $0`
. $scriptDir/../../GlobalValues.in

tmpDir="${CODE_BASE_DIR}/${TEMPORARY_FILES}/${molName}"

rm -rf $tmpDir

# empty the directories after fetching a state
