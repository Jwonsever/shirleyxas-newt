#!/bin/bash

if [[ $# < 1 ]]; then
    echo "Usage: file dir='.' \n file must be tab separated, for sensible output" 
    exit
fi
#File to operate on
sfile=$1

mydir="."
if [[ $# > 1 ]]; then
    mydir=$2
fi

#Run Comparisons and plot all X vs Y
mkdir resultPairs
cd resultPairs

