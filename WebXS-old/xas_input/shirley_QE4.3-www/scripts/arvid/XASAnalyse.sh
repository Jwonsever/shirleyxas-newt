#!/bin/bash

. ./Input_Block.in

# spectral details
SIGMA=0.2
NENER=1000
ELOW=-5
EHIGH=30
NOPr=8
TEMP=0.0

ExecDir="/global/home/users/davidp/dev/shirley_QE4.3-r292/bin"
CALC=relax
REF=atom
CALCGS=GS

TYPES=`echo "$ATOMIC_SPECIES" | awk '{if(NR>1){print $1}}' | tr '\n' ' '`

if [ $# != 1 ]
then
  echo usage: ./XASAnalyze.sh type
  echo   available atom types are:
  echo   $TYPES
  exit
fi
EA=$1   #Arg1: excited Atom of interest
#EAmod=`echo ${ATOM_MASS[$EA]} | sed -e  's/\.//g'`

Atoms=""

#check the type of the excited atom

for i in `seq 1 $NAT` ; do

IA=${ATOM_SYMBOL[$i]}

#Imod=`echo ${ATOM_MASS[$i]} | sed 's/\.//g'`


if [ $EA == $IA ] ; then
if [ ${IND_EXCITATION[$i]} -eq 1 ]; then

Atoms="$Atoms $i"

# Previously used to determine which atom provides the alignment
# This is superceded now by having atomic alignment
#if [ $i -eq $EA ] ; then
#
#EAIndex=`echo -e $Atoms | wc -w`
#
#fi


fi
fi

done

NA=`echo -e $Atoms | wc -w`

#get the Basic reference energies

RefAtom=`echo $Atoms | awk '{print $1}' `
RefAtomw=`seq -w $RefAtom $NAT | head -1`
RefSnap=`echo $N | awk '{print$1}'`

echo "Reference atom calculations from directory:
XAS/$REF/${ATOM_SYMBOL[$RefAtom]}"

ERefGS0=`grep '^!' XAS/$REF/${ATOM_SYMBOL[$RefAtom]}/atom.${ATOM_SYMBOL[$RefAtom]}-GS.scf.out | awk '{print $5}' | tail -1`
ERefES0=`grep '^!' XAS/$REF/${ATOM_SYMBOL[$RefAtom]}/atom.${ATOM_SYMBOL[$RefAtom]}-${CHAPPROX}.scf.out | awk '{print $5}' | tail -1`

RunDir=`pwd`

SpecDir=XAS/Spectrum-${ATOM_SYMBOL[$RefAtom]}
if test ! -d $SpecDir ; then
mkdir $SpecDir
fi
cd $SpecDir

#get energies

:> Spectra
NAVE=0
SpectraTMP=""

#shift spectra


for atom in $Atoms ; do 

  atomw=`seq -w $atom $NAT | head -1`
  XASPrefix=$RunDir/XAS/$CALC/${ATOM_SYMBOL[$atom]}${atom}/$MOLNAME.${ATOM_SYMBOL[$atom]}${atomw}-$CHAPPROX

  #get the specific energies for shift
  EXCH=`grep '^!' ${XASPrefix}.scf.out | awk '{print $5}' | tail -1`
  EGS=`grep '^!' $RunDir/XAS/$CALCGS/$MOLNAME.scf.out | awk '{print $5}' | tail -1`
#  ELUMO=`grep Fermi ${XASPrefix}.scf.out | awk '{print $5}' | tail -1`
  # find the Fermi level (and CBM)
  $PARA_PREFIX -np $NOPr $ExecDir/efermi.x $TEMP ${XASPrefix}.xas.$XAS_ARG $NELEC > ${XASPrefix}.xas.${XAS_ARG}-fermi.out

  ELUMO=`tail -2 ${XASPrefix}.xas.${XAS_ARG}-fermi.out | head -1 | awk '{print $3}'`

  E=`echo | awk -v exch=$EXCH -v erefes0=$ERefES0 -v egs=$EGS -v erefgs0=$ERefGS0 -v elumo=$ELUMO '{print (exch-erefes0-egs+erefgs0)*13.6056923 - elumo}'`

  echo "i : (XCH - XCH0) - (GS - GS0) - LUMO = Delta"
  echo $atom : $EXCH $ERefES0 - $EGS $ERefGS0 $ELUMO = $E

  # check for previous .xas file and save
  for file in `ls $RunDir/XAS/$CALC/${ATOM_SYMBOL[$atom]}${atom}/*.xas 2> /dev/null`; do
    mv $file ${file}-raw
  done

  # run xas_para.x
  $PARA_PREFIX -np $NOPr $ExecDir/xas_para.x $ELOW $EHIGH $NENER $SIGMA $E $ELUMO ${XASPrefix}.xas.$XAS_ARG 2> /dev/null

  XASFile=`basename ${XASPrefix}.xas.$XAS_ARG.xas`
  mv  ${XASPrefix}.xas.$XAS_ARG.xas ./$XASFile

#  Spectra=`sed 1d $XASFile | awk '{print $2}' `
#
#  NOSP=`echo -e "$Spectra" | wc -w`
#
#cat >Spectra-${ATOM_SYMBOL[$atom]}$atomw <<EOF
#$Spectra
#EOF

#cat Spectra-${ATOM_SYMBOL[$atom]}$atomw >> Spectra
paste Spectra $XASFile > $$
mv $$ Spectra
NAVE=$(( $NAVE + 1 ))

done

#dump the energy scale
#grep -v '#' $XASFile | awk '{print $1}' >SCALE

AWKSTR=''
AVESTR='(0'
for a in `seq 1 $NAVE`
do
  b=`echo $a | awk '{print $1*5-3}'`
  AWKSTR="${AWKSTR}, \$$b"
  AVESTR="${AVESTR}+\$$b"
done
AVESTR="${AVESTR})/$NAVE"
echo "{print \$1, ${AVESTR}${AWKSTR}}"
awk "{print \$1, ${AVESTR}${AWKSTR}}" Spectra > Spectrum-Ave-${ATOM_SYMBOL[$atom]}

echo output located in 
echo   $SpecDir/Spectrum-Ave-${ATOM_SYMBOL[$atom]}
exit
