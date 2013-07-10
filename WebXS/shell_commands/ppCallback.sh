#!/bin/bash

pp_id=$1
lband=$2

#Wait till convergence, then tar up the file (later send a request back to the user to know its finished)
converged="no"
while [ "$converged" == "no" ]
do
    stat=`qstat | grep $pp_id | awk 'print $5'`
    if [ "$stat" == "C" ]; then
	converged="yes"
    fi
done

cd $dir
tar -pvczf state.$lband.cube.tar.gz state.$lband.cube

#somehow let app know the job is finished?

exit