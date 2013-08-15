#!/bin/bash

pathname=$1

times=`grep -e "walltime=" "${pathname}/xas.out" | rev | cut -c -8 | rev | sed "s/://g"`



set -- $times
if (( $(echo "$1 $2" | awk '{print ($1 < $2)}') )); then
    echo "1"
else
    echo "0"
fi

