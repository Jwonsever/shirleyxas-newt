#!/bin/bash

#PBS -q nano8
#PBS -l nodes=1:ppn=8:nano
#PBS -l walltime=0:15:00

# number of processors
PPN=`wc -l $PBS_NODEFILE | awk '{print $1}'`
# if you want multiple calculations running side by side, indicate it here
NJOB=1
PPN=`echo $PPN / $NJOB | bc`
echo " number of processors given to each independent calculation $PPN"
CALC=relax

cd $PBS_O_WORKDIR
# the PBS preamble should be removed in order to make this
# script more general
. ./Input_Block.in


# This should be defined automatically to point to the right place
#  either as an environment variable (once loaded as a module)
#  or somehow based on the position of this script
BibDir="/global/home/users/davidp/dev/shirley_QE4.3-r292/scripts/arvid/Bibliothek"  #Directroy of the executable
export BibDir
ExecDir="/global/home/users/davidp/dev/shirley_QE4.3-r292/bin"
MyDir=`pwd`
NodePool="${MyDir}/Nodes"
ScriptName=`echo "$0" | sed -e 's/\.\///g'`

ReVa="${BibDir}/ResetVariables.sh ${MyDir}"
VP="${BibDir}/VarPen.sh" #Executable to change the file TMP_INPUT.in
Re="${BibDir}/Reverend.sh"
GeNo="${BibDir}/GetNode.sh"

#end of declarations
######################################################################################

${BibDir}/Chop.sh $PBS_NODEFILE $PPN        #decompose parent node file
######################################################################################

#XAS start

if test ! -d $MyDir/XAS ; then
mkdir $MyDir/XAS
fi


if test ! -d $MyDir/XAS/$CALC ; then
mkdir $MyDir/XAS/$CALC
fi

# loop over atoms
for i in `seq 1 $NAT` ; do

$ReVa #Create file: TMP_INPUT.in

if [ ${IND_EXCITATION[$i]} -eq 1 ] ; then

if test ! -d $MyDir/XAS/$CALC/${ATOM_SYMBOL[$i]}${i} ; then
mkdir $MyDir/XAS/$CALC/${ATOM_SYMBOL[$i]}${i}
fi

CATOM=`seq -w $i $NAT | head -1`

${VP} TMP_BIN_DIR=$ExecDir TMP_MOLNAME=${MOLNAME}.${ATOM_SYMBOL[$i]}${CATOM}-${CHAPPROX} TMP_wf_collect=.TRUE. TMP_disk_io='high'
${BibDir}/Labeler.sh $i

# Get nodes - or skip atom if error
${GeNo}  $MyDir/XAS/$CALC/${ATOM_SYMBOL[$i]}${i} $NodePool || continue
# Make sure you got them
ls $MyDir/XAS/$CALC/${ATOM_SYMBOL[$i]}${i}/Node-${PBS_JOBID}-*
# Run the calc
echo treating atom ${ATOM_SYMBOL[$i]}$i 
${BibDir}/DoXAS.sh $MyDir/XAS/$CALC/${ATOM_SYMBOL[$i]}${i} $i &

sleep 1
ls $NodePool

fi
done
wait
exit

