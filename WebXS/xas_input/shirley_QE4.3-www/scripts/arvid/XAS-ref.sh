#!/bin/bash

#PBS -q vulcan_batch
#PBS -l nodes=4:ppn=8
#PBS -l walltime=1:00:00

# number of processors
PPN=`wc -l $PBS_NODEFILE | awk '{print $1}'`
# if you want multiple calculations running side by side, indicate it here
#NJOB=1
#PPN=`echo $PPN / $NJOB | bc`
echo " number of processors given to each independent calculation $PPN"

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


function do_atom {

SCFOUT=${CalcDir}/${TYP_SYMBOL}/atom.${TYP_SYMBOL}-GS.scf.out
if [ ! -f $SCFOUT ] || ! grep -q 'convergence has been achieved' $SCFOUT
then

$ReVa   #Create file: TMP_INPUT.in
TMP_ELEC_MIXING_BETA=0.1
${VP} TMP_BIN_DIR=$ExecDir TMP_MOLNAME=atom.${TYP_SYMBOL}-GS TMP_wf_collect=.TRUE. TMP_disk_io='low' TMP_NAT=1 TMP_NELEC=$NELEC TMP_NBND=$NBND TMP_ATOMIC_SPECIES="$TMP_ATOMIC_SPECIES" TMP_ATOMIC_POSITIONS="$TMP_ATOMIC_POSITIONS" TMP_CHAPPROX='' TMP_ELEC_MIXING_BETA=$TMP_ELEC_MIXING_BETA TMP_SPIN=$TMP_SPIN TMP_LDAU=$TMP_LDAU

    ${GeNo}  ${CalcDir}/${TYP_SYMBOL} $NodePool || continue
    ls ${CalcDir}/${TYP_SYMBOL}/Node-${PBS_JOBID}-*

${BibDir}/SCF.sh ${CalcDir}/${TYP_SYMBOL} 1 &

    sleep 1
    ls $NodePool

fi

SCFOUT=${CalcDir}/${TYP_SYMBOL}/atom.${TYP_SYMBOL}-${CHAPPROX}.scf.out
if [ ! -f $SCFOUT ] || ! grep -q 'convergence has been achieved' $SCFOUT
then

$ReVa   #Create file: TMP_INPUT.in
TMP_ELEC_MIXING_BETA=0.01
${VP} TMP_BIN_DIR=$ExecDir TMP_MOLNAME=atom.${TYP_SYMBOL}-${CHAPPROX} TMP_wf_collect=.TRUE. TMP_disk_io='low' TMP_NAT=1 TMP_NELEC=$NELEC TMP_NBND=$NBND TMP_ATOMIC_SPECIES="$TMP_ATOMIC_SPECIES" TMP_ATOMIC_POSITIONS="$TMP_ATOMIC_POSITIONS" TMP_CHAPPROX=${CHAPPROX} TMP_ELEC_MIXING_BETA=$TMP_ELEC_MIXING_BETA TMP_SPIN=$TMP_SPIN TMP_LDAU=$TMP_LDAU
${BibDir}/Labeler.sh 1

    ${GeNo}  ${CalcDir}/${TYP_SYMBOL} $NodePool || continue
    ls ${CalcDir}/${TYP_SYMBOL}/Node-${PBS_JOBID}-*

${BibDir}/SCF.sh ${CalcDir}/${TYP_SYMBOL} 1 &

    sleep 1
    ls $NodePool

fi

}

#end of declarations
######################################################################################

${BibDir}/Chop.sh $PBS_NODEFILE $PPN        #decompose parent node file
######################################################################################
#GS start
# subdirectory where calculations will appear - can be anything you want
CALC=GS

if test ! -d $MyDir/XAS ; then
mkdir $MyDir/XAS
fi


if test ! -d $MyDir/XAS/${CALC} ; then
mkdir $MyDir/XAS/${CALC}
fi
CalcDir=${MyDir}/XAS/${CALC}

SCFOUT=${CalcDir}/${MOLNAME}.scf.out
if [ ! -f $SCFOUT ] || ! grep -q 'convergence has been achieved' $SCFOUT
then

$ReVa   #Create file: TMP_INPUT.in
${VP} TMP_BIN_DIR=$ExecDir TMP_wf_collect=.TRUE. TMP_disk_io='low' TMP_CHAPPROX=''

${GeNo}  ${CalcDir} $NodePool || continue
ls ${CalcDir}/Node-${PBS_JOBID}-*

${BibDir}/SCF.sh ${CalcDir} 1 &

sleep 1
ls $NodePool

fi


wait


# atom GS and XAS
# subdirectory where calculations will appear - can be anything you want
CALC=atom

if test ! -d $MyDir/XAS ; then
mkdir $MyDir/XAS
fi


if test ! -d $MyDir/XAS/${CALC} ; then
mkdir $MyDir/XAS/${CALC}
fi
CalcDir=${MyDir}/XAS/${CALC}

# loop over types
for ityp in `seq 1 $NTYP`; do
  DO_TYP[$ityp]=0
  ityp1="$(( ${ityp} + 1 ))"

  TYP_SYMBOL=`echo "$ATOMIC_SPECIES" | awk -v line=$ityp1 '{if(line==NR){print $1}}'`
  for i in `seq 1 $NAT` ; do
    if [ ${ATOM_SYMBOL[$i]} == $TYP_SYMBOL ] ; then
      if [ ${IND_EXCITATION[$i]} -eq 1 ] ; then
        DO_TYP[$ityp]=1
      fi
    fi
  done

  # if we are interested in this atom type then do an atomic calc
  if [ ${DO_TYP[$ityp]} -eq 1 ] ; then

    echo type $ityp

    TYP_Z=`echo "$ATOMIC_SPECIES" | head -$ityp1 | tail -1 | awk '{print $3}'`
    echo $PSEUDO_DIR/$TYP_Z
    if [ ! -f $PSEUDO_DIR/$TYP_Z ] ; then
      echo Pseudopotential not found: $PSEUDO_DIR/$TYP_Z
      exit
    fi
    NELEC=`grep 'Z valence' $PSEUDO_DIR/$TYP_Z | awk '{print $1}'`
    # Determine the number of bands
    NBND=`echo "($NELEC*0.5*1.2)/1" | bc`
    NBND1=`echo "($NELEC*0.5)/1+4" | bc`
    if [[ $NBND1 -gt $NBND ]] ; then
      NBND=$NBND1
    fi
    if [[ $NBND -lt $PPN ]] ; then
      NBND=$PPN
    fi


    TMP_ATOMIC_SPECIES="`echo "$ATOMIC_SPECIES" | head -1`
`echo "$ATOMIC_SPECIES" | head -$ityp1 | tail -1`"
    TMP_ATOMIC_POSITIONS="ATOMIC_POSITIONS (angstrom)
$TYP_SYMBOL 0.0 0.0 0.0"

    if [[ -n $SPIN ]]; then
      sedstr="/starting_magnetization($ityp)/s/.*starting_magnetization($ityp) *= *\([+-]*[0-9]*\.*[0-9]*\).*/starting_magnetization(1)=\1/p"
      TMP_SPIN=`echo $SPIN | sed -n "$sedstr"`
      if [[ -n $TMP_SPIN ]]; then
        TMP_SPIN=`echo $SPIN | sed -n '/nspin/s/.*\(nspin *= *[0-9]*\).*/\1/p'
echo $TMP_SPIN`
      fi
    fi

    if [[ -n $LDAU ]]; then
      sedstr="/Hubbard_U($ityp)/s/.*Hubbard_U($ityp) *= *\([+-]*[0-9]*\.*[0-9]*\).*/Hubbard_U(1)=\1/p"
      TMP_LDAU=`echo $LDAU | sed -n "$sedstr"`
      if [[ -n $TMP_LDAU ]]; then
        TMP_LDAU=`echo $LDAU | sed -n '/lda_plus_u/s/.*\(lda_plus_u *= *\.[a-zA-Z]*\.\).*/\1/p'
echo $TMP_LDAU`
      fi
    fi

    echo $TYP_SYMBOL
    echo $NELEC $NBND
    echo "$TMP_ATOMIC_SPECIES"
    echo "$TMP_ATOMIC_POSITIONS"
    echo "$TMP_SPIN"
    echo "$TMP_LDAU"

    if test ! -d ${CalcDir}/${TYP_SYMBOL} ; then
      mkdir ${CalcDir}/${TYP_SYMBOL}
    fi

    do_atom 

  fi

done

wait
exit

