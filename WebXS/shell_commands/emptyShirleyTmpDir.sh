#!/bin/bash

# Load Global Variables
scriptDir=`dirname $0`
. $scriptDir/../../GlobalValues.in

cd $CODE_BASE_DIR/$TEMPORARY_FILES
find . -atime +30 -exec rm {} \;
