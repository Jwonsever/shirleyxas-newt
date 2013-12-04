#!/bin/bash -l

MOLNAME="$2"

dir="${1}/${MOLNAME}"
echo $dir
rm -rf $dir