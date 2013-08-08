#!/bin/bash -l

MOLNAME="$2"

echo $MOLNAME

dir="${1}/${MOLNAME}"

function recMkdir {
    echo $1

    if ( mkdir $1 ); then
	return
    else 
	recMkdir `dirname $1`
	mkdir $1
	return
    fi
}

recMkdir $dir