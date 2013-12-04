#!/bin/bash

if [ $# != 1 ]
then
  echo usage AveSpectra.sh N
  exit
fi

. ../../Input_Block.in

EA=$1   #Arg1: excited Atom of interest
EAmod=`echo ${ATOM_MASS[$EA]} | sed -e  's/\.//g'`

Atoms=""

#check the type of the excited atom

for i in `seq 1 $NAT` ; do

Imod=`echo ${ATOM_MASS[$i]} | sed 's/\.//g'`


if [ $EAmod -eq $Imod ] ; then
if [ ${IND_EXCITATION[$i]} -eq 1 ]; then

Atoms="$Atoms $i"

if [ $i -eq $EA ] ; then

EAIndex=`echo -e $Atoms | wc -w`

fi


fi
fi

done

if test ! -d ../XAS ; then 

exit

fi

NA=`echo -e $Atoms | wc -w`

#get the Basic reference energies

RefAtom=`echo $Atoms | awk '{print $1}' `
RefAtomw=`seq -w $RefAtom $NAT | head -1`
RefSnap=`echo $N | awk '{print$1}'`



if test ! -d ../XAS/${ATOM_SYMBOL[$RefAtom]} ; then

exit

fi

NOSP=`wc -w ../XAS/${ATOM_SYMBOL[$RefAtom]}/Spectra | awk '{print $1}'`

#overall averaged spectra

NASP=`echo $N | wc -w`

echo $NOSP $NASP $NA

rm -f ../XAS/Spectra-${ATOM_SYMBOL[$RefAtom]}

for NN in $N ; do
cat ../XAS/${ATOM_SYMBOL[$RefAtom]}/Spectra >>../XAS/Spectra-${ATOM_SYMBOL[$RefAtom]}
done

ScaleRef=../XAS/${ATOM_SYMBOL[$RefAtom]}/SCALE

cat > ../XAS/TMP.in<<EOF

Sp=load("-ascii","../XAS/Spectra-${ATOM_SYMBOL[$RefAtom]}");
Scale=load("-ascii","$ScaleRef");

ne=$NOSP/$NA
S=sum(reshape(Sp,ne,$NASP*$NA),2)./($NASP*$NA)
S2=sum(reshape(Sp.*Sp,ne,$NASP*$NA),2)./($NASP*$NA)
S2=sqrt(S2.-(S.*S))
All=[Scale S S2];

save -ascii ../XAS/SpectraAll${ATOM_SYMBOL[$1]} All
EOF

octave ../XAS/TMP.in | >/dev/null

exit

