#!/bin/bash

pathname=$1

#Jump into dir
cd $1
pathname=`pwd`
jobname=`basename $pathname`

pseudos=`grep -e "Pseudopotential for " "${pathname}/XAS/${jobname}_0/Input_Block.in" | awk '{print $3}'`

echo $pseudos

if [ -n "${pseudos}" ]; then
    echo "1"
else
    echo "0"
fi
