#!/bin/bash

scriptDir=`dirname $0`

pathname=$1
for f in $pathname/*;
do
    myres=`$scriptDir/exceededWalltime.sh "${f}" 2>/dev/null`
    if [ "$myres" == "1" ]; then
	echo `basename $f`
    fi
done


