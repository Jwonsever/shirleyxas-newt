#!/bin/bash
#PBS -q vulcan_batch
#PBS -l nodes=2:ppn=8:vulcan
#PBS -l walltime=24:00:00


cd $PBS_O_WORKDIR
# the PBS preamble should be removed in order to make this
# script more general
. ./Input_Block.in


# This should be defined automatically to point to the right place
#  either as an environment variable (once loaded as a module)
#  or somehow based on the position of this script
BD="/global/home/users/davidp/arvid/scripts/Bibliothek"  #Directory of the executables

#PathNodeFile=`echo "$PBS_NODEFILE" | sed -e 's/\/\//\//g'`
ScriptName=`echo "$0" | sed -e 's/\.\///g'`
#PPN=`grep '#PBS -q nano*' < $ScriptName | sed -n "1p" | sed -e 's/#PBS -q nano//g'`
PPN=16
MyDir=`pwd`
NodePool="${MyDir}/Nodes"

ReVa="${BD}/ResetVariables.sh ${MyDir}"
VP="${BD}/VarPen.sh" #Executable to change the file TMP_INPUT.in
Re="${BD}/Reverend.sh"
GeNo="${BD}/GetNode.sh"

#end of declarations
######################################################################################

${BD}/Chop.sh $PBS_NODEFILE $PPN 	#decompose parent node file
$ReVa	#Create file: TMP_INPUT.in


#GS_MD
######################################################################################

if test ! -d $MyDir/GS ; then
mkdir $MyDir/GS
fi

$VP TMP_NSTEP=1000 TMP_MOLNAME=${MOLNAME}.MD TMP_wf_collect=.true. TMP_disk_io=default

${BD}/MD-metal.sh ${MyDir}/GS 0
exit

#get the last atomic positions of the MD-GS calculation
ATOM_POS=
ATOM_POS=`grep -A $NAT 'ATOMIC_POSITIONS' < ${MyDir}/GS/${MOLNAME}.MD.out |sed -e '/--/d' -e '/ATOMIC_POSITIONS/d' | sed -n "$((((1000 - 1 )*$NAT) + 1 )),$(( 1000 * $NAT  ))p"`
#ATOM_POS=`echo "$ATOM_POS" |awk "{if(NR>$(( ($NN-1)*$NAT )) && NR<=$(( ($NN)*$NAT ))){print }}" |awk '{print $1, $2, $3, $4}'`
ATOM_POS="ATOMIC_POSITIONS (angstrom)
$ATOM_POS"
 
 
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



$VP TMP_NSTEP=4000 TMP_ION_TEMPERATURE='not_controlled' TMP_RM='restart' TMP_wf_collect=.false. TMP_disk_io=default

cp ./GS/${MOLNAME}.MD.out ./GS/${MOLNAME}.MD.out_COPY
${BD}/MD.sh ${MyDir}/GS 0
${Re} 1 ./GS/${MOLNAME}.MD.out_COPY ./GS/${MOLNAME}.MD.out


if test ! -d ${MyDir}/SCF ; then
mkdir ${MyDir}/SCF
fi

for NN in $N ; do

#SCF
#######################################################################################

if test ! -d ${MyDir}/SCF/NSTEP_$NN ; then
mkdir ${MyDir}/SCF/NSTEP_$NN
fi


for i in `seq 1 $NAT` ; do
if [ ${IND_EXCITATION[$i]} -eq 1 ] ; then



#get the last atomic positions of the MD-GS calculation
ATOM_POS=
ATOM_POS=`grep -A $NAT 'ATOMIC_POSITIONS' < ${MyDir}/GS/${MOLNAME}.MD.out |sed -e '/--/d' -e '/ATOMIC_POSITIONS/d' | sed -n "$(((($NN - 1 )*$NAT) + 1 )),$(( $NN * $NAT  ))p"`
#ATOM_POS=`echo "$ATOM_POS" |awk "{if(NR>$(( ($NN-1)*$NAT )) && NR<=$(( ($NN)*$NAT ))){print }}" |awk '{print $1, $2, $3, $4}'`
ATOM_POS="ATOMIC_POSITIONS (angstrom)
$ATOM_POS"


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



if test ! -d "${MyDir}/SCF/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i}" ; then
mkdir ${MyDir}/SCF/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i}
fi

CATOM=`seq -w $i $NAT | head -1`
${VP} TMP_MOLNAME=${MOLNAME}.${ATOM_SYMBOL[$i]}${CATOM}-${CHAPPROX} TMP_wf_collect=.TRUE. TMP_disk_io='high'


${BD}/Labeler.sh $i
${GeNo} ${MyDir}/SCF/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i} $NodePool
${BD}/SCF.sh  ${MyDir}/SCF/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i} 1 &



fi
done
done

wait

for NN in $N ; do


#MD-ES
#######################################################################################

if test ! -d ${MyDir}/ES ; then
mkdir ${MyDir}/ES/
fi

if test ! -d ${MyDir}/ES/NSTEP_$NN ; then
mkdir ${MyDir}/ES/NSTEP_$NN
fi

for i in `seq 1 $NAT` ; do

if [ ${IND_EXCITATION[$i]} -eq 1 ] ; then

CATOM=`seq -w $i $NAT | head -1`
${VP} TMP_MOLNAME=${MOLNAME}.MD.N${NN}_${ATOM_SYMBOL[$i]}${i} TMP_wf_collect=.FALSE. TMP_disk_io=default

#Copy the results from the SCF calculation
cp -r ${MyDir}/SCF/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i}/ ${MyDir}/ES/NSTEP_$NN/

#Create the pwscf


#get the last atomic positions of the MD-GS calculation
ATOM_POS=
ATOM_POS=`grep -A $NAT 'ATOMIC_POSITIONS' < ${MyDir}/GS/${MOLNAME}.MD.out |sed -e '/--/d' -e '/ATOMIC_POSITIONS/d' | sed -n "$(((($NN - 1 )*$NAT) + 1 )),$(( $NN * $NAT  ))p"`
#ATOM_POS=`echo "$ATOM_POS" |awk "{if(NR>$(( ($NN-1)*$NAT )) && NR<=$(( ($NN)*$NAT ))){print }}" |awk '{print $1, $2, $3, $4}'`
ATOM_POS="ATOMIC_POSITIONS (angstrom)
$ATOM_POS"


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


MD_ES_NSTEP=$(( $NSTEP + $XCH_STEP ))
${VP} TMP_NSTEP=$MD_ES_NSTEP "TMP_RM='restart'" "TMP_ION_TEMPERATURE='not_controlled'"
${BD}/Labeler.sh $i

#Start MD_Es

${GeNo} ${MyDir}/ES/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i} $NodePool


${BD}/GetPWSCF.md.sh $NN ${MyDir}/GS/${MOLNAME}.MD.out ${MyDir}/ES/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i}/pwscf.md "${MyDir}/SCF/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i}/${MOLNAME}.${ATOM_SYMBOL[$i]}${CATOM}-${CHAPPROX}.SCF.out"

${BD}/MD.sh  ${MyDir}/ES/NSTEP_$NN/${ATOM_SYMBOL[$i]}${i} 1 &


fi
done
done

wait


exit
