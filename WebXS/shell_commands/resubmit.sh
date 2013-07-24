#!/bin/bash -l
#Reruns XAS, Assumes REF finished, and sets ANAL and STATE to rerun IF it finishes.

dir=$1

cd $dir
echo `pwd`


#Is this a rerun? Save the preveious xas.out. 
if [[ -f ./xas.out ]]; then
    cat ./xas.out >> xas.out.old
fi

xas_id=`qsub xas.qscript `
anal_id=`qsub -W depend=afterok:${xas_id}@hopper11 anal.qscript`
state_id=`qsub -W depend=afterok:${anal_id}@hopper11 state.qscript`
