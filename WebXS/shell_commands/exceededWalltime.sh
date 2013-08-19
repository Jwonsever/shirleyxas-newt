#!/bin/bash

pathname=$1

times=`grep -e "walltime=" "${pathname}/xas.out" | rev | cut -c -8 | rev | sed "s/://g"`

lastline=`tail -n 1 ${pathname}/xas.out` 
if [ `grep -c "Resubmitted" $lastline` -ne 0 ]; then
    echo "0"
    exit
fi

set -- $times
if (( $(echo "$1 $2" | awk '{print ($1 < $2)}') )); then
    echo "1"
else
    echo "0"
fi

exit