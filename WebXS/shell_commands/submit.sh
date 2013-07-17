#!/bin/bash -l
#Creates Input_Block.in and prepends PBS headers to scripts. Then submits job.

# Load Global Variables
scriptDir=`dirname $0`
. $scriptDir/../../GlobalValues.in

echo "Total number of arg passed: $#"

#Redo this with optional argumnts
dir="${1}/${2}"
MOLNAME="$2"
inputs=$3
numberNodes=$4
procsPerPool=$5
machine=$6
queue=$7
walltime=$8

#Added option to use own account hours.
account=$9

shift
inputLocation="${CODE_BASE_DIR}/${CODE_LOC}/${XAS_INPUTS}/Input_Block.in"
customInputLocation=0

if (( $(echo "$# > 8" | bc -l) )); then
    customInputLocation=$9
fi

#Run selection of ExcitedStates?
xstateflag=1

cd $dir
cp $inputLocation ./Input_Block.in

#ATM it writes all three.  tofix. todo.

if [ $customInputLocation ] ; then
    echo "grabbing $customInputLocation"	
    cat $customInputLocation >> ./Input_Block.in
fi

echo -e ${inputs} >> ./Input_Block.in


xasPBS="#!/bin/bash\n"
xasPBS+="#PBS -q ${queue}\n"
ppPBS+="#PBS -l walltime=${walltime}\n"
xasPBS+="#PBS -V\n"

#Use Account,if not default.
xasPBS+="#PBS -A ${account}\n"

refPBS="$xasPBS"
analPBS="$xasPBS"
statePBS="$xasPBS"

xasPBS+="#PBS -e xas.err\n"
xasPBS+="#PBS -o xas.out\n"
refPBS+="#PBS -e ref.err\n"
refPBS+="#PBS -o ref.out\n"
analPBS+="#PBS -e anal.err\n"
analPBS+="#PBS -o anal.out\n"
statePBS+="#PBS -e state.err\n"
statePBS+="#PBS -o state.out\n"

xasPBS+="#PBS -N ${MOLNAME}-XAS\n"
refPBS+="#PBS -N ${MOLNAME}-REF\n"
analPBS+="#PBS -N ${MOLNAME}-ANALYSE\n"
statePBS+="#PBS -N ${MOLNAME}-STATE\n"

if [ "$machine" == "carver" ]; then
    nAllocNodes=$(echo "$numberNodes" | bc)
    nAllocRef=$nAllocNodes
    xasPBS+="#PBS -l nodes=${nAllocNodes}:ppn=${procsPerPool}\n\n"
    refPBS+="#PBS -l nodes=${nAllocRef}:ppn=${procsPerPool}\n\n"
    analPBS+="#PBS -l nodes=1:ppn=${procsPerPool}\n\n"
    statePBS+="#PBS -l nodes=1:ppn=${procsPerPool}\n\n"

else 
    xasWidth=$(echo "$numberNodes*$procsPerPool" | bc)
    refWidth=$(echo "$procsPerPool" | bc)
    analWidth=$(echo "$procsPerPool" | bc)
    stateWidth=$(echo "$procsPerPool" | bc)
    xasPBS+="#PBS -l mppnppn=${procsPerPool}\n"
    xasPBS+="#PBS -l mppwidth=${xasWidth}\n\n"
    refPBS+="#PBS -l mppnppn=${procsPerPool}\n"
    refPBS+="#PBS -l mppwidth=${refWidth}\n\n"
    analPBS+="#PBS -l mppnppn=${procsPerPool}\n"
    analPBS+="#PBS -l mppwidth=${analWidth}\n"  
    statePBS+="#PBS -l mppnppn=${procsPerPool}\n"
    statePBS+="#PBS -l mppwidth=${stateWidth}\n"
fi

xasPBS+="cd ${dir}\n"
xasPBS+="export NO_STOP_MESSAGE=1\n\n"
refPBS+="cd ${dir}\n"
refPBS+="export NO_STOP_MESSAGE=1\n\n"

echo -e ${xasPBS} > ./xas.qscript
echo -e ${refPBS} > ./ref.qscript

cat $CODE_BASE_DIR/$CODE_LOC/$XAS_INPUTS/XAS-xyz.sh >> ./xas.qscript
cat $CODE_BASE_DIR/$CODE_LOC/$XAS_INPUTS/XAS-xyz-ref.sh >> ./ref.qscript

## Submit xas and xas-ref
ref_id=`qsub ref.qscript `
xas_id=`qsub xas.qscript `

echo -e ${xas_id} > ./jobid.txt

#Kill this line to match new qsub reqs.
#analPBS+="#PBS -W depend=afterok:${xas_id}@hopper11\n\n"
analPBS+="cd ${dir}\n"
analPBS+="export NO_STOP_MESSAGE=1\n\n"

echo -e ${analPBS} > ./anal.qscript

qstat -f $ref_id | grep $account >> stats.txt

cat $CODE_BASE_DIR/$CODE_LOC/$XAS_INPUTS/XASAnalyse-xyz.sh >> ./anal.qscript

## Submit xas-analyse, dependent on successful completion of xas.sh
#/usr/common/nsg/bin/qsub anal.qscript (Previous version)
anal_id=`qsub -W depend=afterok:${xas_id}@hopper11 anal.qscript`

if [ $xstateflag == 1 ]; then
    statePBS+="cd ${dir}\n"
    statePBS+="export NO_STOP_MESSAGE=1\n\n"

    echo -e ${statePBS} > ./state.qscript
    
    #So it can find other scripts
    echo -e "scriptDir=${scriptDir}" >> ./state.qscript

    cat $CODE_BASE_DIR/$CODE_LOC/$SERVER_SCRIPTS/runExcitedStates.sh >> ./state.qscript
    state_id=`qsub -W depend=afterok:${anal_id}@hopper11 state.qscript`
fi

exit
