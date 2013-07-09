#!/bin/bash -l
#deletes .cube files

molName=$1

tmpDir="/project/projectdirs/mftheory/www/james-xs/Shirley-data/tmp/${molName}"

rm -rf $tmpDir

# empty the directories
