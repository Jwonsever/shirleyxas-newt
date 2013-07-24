#!/bin/bash

pathname=$1

#Jump into dir
cd $1
pathname=`pwd`
jobname=`basename $pathname`

mpierr=` grep -n -A 1 -m 1 -e "mpi" *`

echo $mpierr
