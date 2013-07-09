#!/bin/bash

pathname=$1

pseudos=`grep -e "Pseudopotential: command not found" "${pathname}/xas.err"`

if [ -n "${pseudos}" ]; then
    echo "1"
else
    echo "0"
fi

