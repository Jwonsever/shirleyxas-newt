#!/bin/bash

#PBS -q vulcan_batch
#PBS -l nodes=30:ppn=8:vulcan
#PBS -l walltime=12:00:00

cd $PBS_O_WORKDIR
# the PBS preamble should be removed in order to make this
# script more general
. ./Input_Block.in


# This should be defined automatically to point to the right place
#  either as an environment variable (once loaded as a module)
#  or somehow based on the position of this script
BibDir="/global/home/users/davidp/arvid/scripts/Bibliothek"  #Directroy of the executable
export BibDir
ExecDir="/global/home/users/davidp/dev/shirley/bin"
MyDir=`pwd`
NodePool="${MyDir}/Nodes"
ScriptName=`echo "$0" | sed -e 's/\.\///g'`
#PPN=`grep '#PBS -q nano*' < $ScriptName | sed -n "1p" | sed -e 's/#PBS -q nano//g'`
PPN=16

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


for NN in $N ; do

if test ! -d $MyDir/XAS/NSTEP_$NN ; then
mkdir $MyDir/XAS/NSTEP_$NN
fi

# loop over atoms
for i in `seq 1 $NAT` ; do
echo treating atom $i excitation ${IND_EXCITATION[$i]}
if [ ${IND_EXCITATION[$i]} -eq 1 ] ; then

$ReVa   #Create file: TMP_INPUT.in

if test ! -d $MyDir/XAS/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i} ; then
mkdir $MyDir/XAS/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i}
fi

#get the last atomic positions of the MD-GS calculation
ATOM_POS=
ATOM_POS=`grep -A $NAT 'ATOMIC_POSITIONS' < ${MyDir}/GS/${MOLNAME}.MD.out |sed -e '/--/d' | sed -n "$(((($NN - 1 )*($NAT + 1)) + 1 )),$(( $NN * ($NAT + 1)  ))p"`

#save the atomic positions in TMP_INPUT.in file
StartLine=
StartLine=`grep -n "TMP_ATOMIC_POSITIONS" ./TMP_INPUT.in | sed -e 's/:/ /g'| awk '{print $1}' `
Len=
Len=`echo "$ATOM_POS" | wc -l`

sed -i "$StartLine,$(( $StartLine -1 + $Len ))d" "./TMP_INPUT.in"
sed -i "/^$/d" "./TMP_INPUT.in"    #removes all blank lines

cat >> ./TMP_INPUT.in <<EOF
TMP_ATOMIC_POSITIONS="$ATOM_POS"
EOF


CATOM=`seq -w $i $NAT | head -1`

${VP} TMP_BIN_DIR=$ExecDir TMP_MOLNAME=${MOLNAME}.${ATOM_SYMBOL[$i]}${CATOM}-${CHAPPROX} TMP_wf_collect=.TRUE. TMP_disk_io='high'
${BibDir}/Labeler.sh $i

SCFOUT=$MyDir/XAS/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i}/${MOLNAME}.${ATOM_SYMBOL[$i]}${CATOM}-${CHAPPROX}.scf.out
NSCFOUT=$MyDir/XAS/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i}/${MOLNAME}.${ATOM_SYMBOL[$i]}${CATOM}-${CHAPPROX}.nscf.out
BASISOUT=$MyDir/XAS/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i}/${MOLNAME}.${ATOM_SYMBOL[$i]}${CATOM}-${CHAPPROX}.basis.out
HAMOUT=$MyDir/XAS/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i}/${MOLNAME}.${ATOM_SYMBOL[$i]}${CATOM}-${CHAPPROX}.ham.out
XASOUT=$MyDir/XAS/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i}/${MOLNAME}.${ATOM_SYMBOL[$i]}${CATOM}-${CHAPPROX}.xas.out

XASJOB=''
if [ -f $SCFOUT ] && grep -q 'convergence has been achieved' $SCFOUT
then

  if [ -f $XASOUT ] && grep -q 'end shirley_xas' $XASOUT
  then
    continue
  fi

  if [ -f $HAMOUT ] && grep -q 'fft_scatter' $HAMOUT
  then
    XASJOB=1
  fi
  if [ -z $XASJOB ] && [ -f $BASISOUT ] && grep -q 'fft_scatter' $BASISOUT
  then
    XASJOB=2
  fi
  if [ -z $XASJOB ] && [ -f $NSCFOUT ] && grep -q 'fft_scatter' $NSCFOUT
  then
    XASJOB=3
  fi
  if [ -z $XASJOB ]
  then
    XASJOB=4
  fi

else
  XASJOB=5
fi

rm -f $MyDir/XAS/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i}/Node*
${GeNo}  $MyDir/XAS/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i} $NodePool
ls $MyDir/XAS/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i}/Node*
${BibDir}/DoXAS.sh $MyDir/XAS/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i} $i $XASJOB &

sleep 1
ls $NodePool

fi
done
done
wait
exit

