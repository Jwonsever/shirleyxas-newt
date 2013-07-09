#!/bin/bash

homedir="/project/projectdirs/mftheory/www/james-xs/Shirley-data/tmp";

cd $homedir;

infile=${1}
outfile=${2}

echo "copy ${infile} ${outfile}" | ../../WebXS/gdis-0.90/gdis

#Run Cleanup
rm $infile
chmod 777 $infile$outfile