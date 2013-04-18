#!/bin/bash -l
#Creates Input_Block.in and prepends PBS headers to scripts. Then submits job.
#Version 11/1

MOLNAME="$2"
inputs=$3
numberNodes=$4
procsPerPool=$5
machine=$6
queue=$7
walltime=$8

#account=$9
#Add option to use own account hours.

dir="${1}/${MOLNAME}"
cd $dir

cp /project/projectdirs/als/www/jack-area/WebXS/xas_input/Input_Block.in .
#mv Input_Block2.in Input_Block.in 
echo -e ${inputs} >> ./Input_Block.in

xasPBS="#!/bin/bash\n"
xasPBS+="#PBS -q ${queue}\n"
xasPBS+="#PBS -V\n"

refPBS="$xasPBS"
analPBS="$xasPBS"

xasPBS+="#PBS -e xas.err\n"
xasPBS+="#PBS -o xas.out\n"
refPBS+="#PBS -e ref.err\n"
refPBS+="#PBS -o ref.out\n"
analPBS+="#PBS -e anal.err\n"
analPBS+="#PBS -o anal.out\n"

xasPBS+="#PBS -N ${MOLNAME}-XAS\n"
refPBS+="#PBS -N ${MOLNAME}-REF\n"
analPBS+="#PBS -N ${MOLNAME}-ANALYSE\n"

if [ "$machine" == "carver" ]; then
    nAllocNodes=$(echo "$numberNodes" | bc)
    nAllocRef=$nAllocNodes
    xasPBS+="#PBS -l nodes=${nAllocNodes}:ppn=${procsPerPool}\n\n"
    refPBS+="#PBS -l nodes=${nAllocRef}:ppn=${procsPerPool}\n\n"
    analPBS+="#PBS -l nodes=1:ppn=${procsPerPool}\n\n"
else 
    xasWidth=$(echo "$numberNodes*$procsPerPool" | bc)
    refWidth=$(echo "$numberNodes*$procsPerPool" | bc)
    analWidth=$(echo "$procsPerPool" | bc)
    xasPBS+="#PBS -l mppnppn=${procsPerPool}\n"
    xasPBS+="#PBS -l mppwidth=${xasWidth}\n\n"
    refPBS+="#PBS -l mppnppn=${procsPerPool}\n"
    refPBS+="#PBS -l mppwidth=${refWidth}\n\n"
    analPBS+="#PBS -l mppnppn=${procsPerPool}\n"
    analPBS+="#PBS -l mppwidth=${analWidth}\n"
fi

xasPBS+="cd ${dir}\n"
xasPBS+="export NO_STOP_MESSAGE=1\n\n"
refPBS+="cd ${dir}\n"
refPBS+="export NO_STOP_MESSAGE=1\n\n"

echo -e ${xasPBS} > ./xas.qscript
echo -e ${refPBS} > ./ref.qscript

cat /project/projectdirs/als/www/jack-area/WebXS/xas_input/XAS-xyz.sh >> ./xas.qscript
cat /project/projectdirs/als/www/jack-area/WebXS/xas_input/XAS-xyz-ref.sh >> ./ref.qscript

## Submit xas and xas-ref
ref_id=`/usr/common/nsg/bin/qsub ref.qscript `
xas_id=`/usr/common/nsg/bin/qsub xas.qscript `

analPBS+="#PBS -W depend=afterok:${xas_id}@sdb\n\n"
analPBS+="cd ${dir}\n"
analPBS+="export NO_STOP_MESSAGE=1\n\n"

echo -e ${analPBS} > ./anal.qscript

qstat -f $ref_id | grep Account >> stats.txt

cat /project/projectdirs/als/www/jack-area/WebXS/xas_input/XASAnalyse-xyz.sh >> ./anal.qscript

## Submit xas-analyse, dependent on successful completion of xas.sh
/usr/common/nsg/bin/qsub anal.qscript

exit
