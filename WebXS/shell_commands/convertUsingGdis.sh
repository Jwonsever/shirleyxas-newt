#!/bin/bash

infile=${1}
outfile=${2}
fullfile=$infile + $outfile

echo "copy ${infile} ${outfile}" | ./gdis-0.90/gdis >& /dev/null

#Run Cleanup
rm $infile