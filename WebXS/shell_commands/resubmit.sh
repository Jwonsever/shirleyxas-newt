#!/bin/bash -l
#Reruns XAS, Assumes REF finished, and sets ANAL and STATE to rerun IF it finishes.

dir=$1

cd $dir
echo `pwd`


#Is this a rerun? Save the preveious xas.out, and write resub flag. 
if [[ -f ./xas.out ]]; then
    cp ./xas.out >> xas.out.old
    echo "Resubmitted" >> xas.out
fi

xas_id=`qsub xas.qscript `
anal_id=`qsub -W depend=afterok:${xas_id} anal.qscript`
state_id=`qsub -W depend=afterok:${anal_id} state.qscript`
