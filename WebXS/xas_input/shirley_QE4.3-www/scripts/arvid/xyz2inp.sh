#!/bin/bash

echo "# xyz2inp - convert XYZ file to Input_Block.in"

if [ $# -lt 2 ] || [ $2 != 'angstrom' ] && [ $2 != 'crystal' ]
then
  echo "usage: xyz2inp file.xyz [angstrom|crystal] {elements}"
  exit
fi

# Source this file
. ./Input_Block.in

BibDir="$SHIRLEY_ROOT/scripts/arvid/Bibliothek"
#USed to read from Bib-dir, better abstaction if read from pseudos.
. ${PSEUDO_DIR}/periodic.table $PSEUDO_FUNCTIONAL

XYZ=$1
shift
units=$1
shift
elements=$@


# begin
NATOM=`sed -n '1p' $XYZ | awk '{print $1}'`

ATOMIC_POSITIONS="ATOMIC_POSITIONS $units"

for i in `seq 1 $NATOM`
do
  ATOM_SYMBOL[$i]=`sed -n "$((i + 2))p" $XYZ`
  line=`sed -n "$((i + 2))p" $XYZ`
  ATOM_SYMBOL[$i]=`echo $line | awk '{print $1}'`
  ATOMIC_POSITIONS="$ATOMIC_POSITIONS
$line"
  # atomic number
  AN=`get_atomicnumber ${ATOM_SYMBOL[$i]}`
  # mass
  ATOM_MASS[$i]=${MASS[$AN]}
  # excitation
  if [[ -n $elements ]]; then
    for el in $elements; do
      if [ ${ATOM_SYMBOL[$i]} = $el ] || [ ${ATOM_SYMBOL[$i]}$i = $el ]
      then
        IND_EXCITATION[$i]='1'
        break
      else
        IND_EXCITATION[$i]='0'
      fi
    done
  else
    if [ ${ATOM_SYMBOL[$i]} != 'H' ]
    then
      IND_EXCITATION[$i]='1'
    else
      IND_EXCITATION[$i]='0'
    fi
  fi
done

ATOMIC_SPECIES="ATOMIC_SPECIES"
SPECIES=`echo ${ATOM_SYMBOL[*]} | tr ' ' '\n' | sort | uniq`
i=0
for S in $SPECIES
do
  i=$((i+1))
  A=`get_atomicnumber $S`
  ATOMIC_SPECIES="$ATOMIC_SPECIES
$S ${MASS[$A]} ${PSEUDO[$A]}"
done

# Determine the number of atoms
NAT=`echo "$ATOMIC_POSITIONS" | wc -l`
NAT=$(( $NAT - 1 ))
# Determine the number of types
NTYP=`echo "$ATOMIC_SPECIES" | wc -l`
NTYP=$(( $NTYP - 1 ))
# Determine charges of ions
for ityp in `seq 1 $NTYP` ; do
  ityp1=$(( $ityp + 1 ))
  Label[$ityp]=`echo "$ATOMIC_SPECIES" | head -$ityp1 | tail -1 | awk '{print $1}'`
  PP=`echo "$ATOMIC_SPECIES" | head -$ityp1 | tail -1 | awk '{print $3}'`
  if [ ! -f $PSEUDO_DIR/$PP ] ; then
    echo Pseudopotential for ${Label[$ityp]} not found: $PSEUDO_DIR/$PP
    echo "$ATOMIC_SPECIES"
    exit 1
  fi
  Zion[$ityp]=`grep 'Z valence' $PSEUDO_DIR/$PP | awk '{print $1}'`
done
# Determine types of atoms
for i in `seq 1 $NAT` ; do
  ip1=$(( $i + 1 ))
  Lab=`echo "$ATOMIC_POSITIONS" | sed -n "${ip1}p" | awk '{print $1}'`
  for ityp in `seq 1 $NTYP` ; do
    if [[ $Lab == ${Label[$ityp]} ]] ; then
      AtomType[$i]=$ityp
      break
    fi
  done
done
# Determine the number of electrons
NELEC=0.0
for i in `seq 1 $NAT` ; do
  NELEC=`echo "($NELEC + ${Zion[${AtomType[$i]}]})" | bc`
done
# Determine the number of bands
NBND=`echo "($NELEC*0.5*1.2)/1" | bc`
NBND1=`echo "($NELEC*0.5)/1+4" | bc`
if [[ $NBND1 -gt $NBND ]] ; then
  NBND=$NBND1
fi


# dump

echo
echo "NAT=$NAT"
echo "NTYP=$NTYP"
echo "NELEC=$NELEC"
echo "NBND=$NBND"
echo
echo "ATOMIC_SPECIES=\"$ATOMIC_SPECIES\""
echo "ATOMIC_POSITIONS=\"$ATOMIC_POSITIONS\""
echo
echo '#Process information----------------------------------------'
# ATOM_SYMBOL
echo
echo '#---------------------Atomic symbols-------------------------'
echo
for i in `seq 1 $NATOM`
do
  echo "ATOM_SYMBOL[$i]='${ATOM_SYMBOL[$i]}'"
done
# ATOM_MASS
echo
echo '#---------------------Atomic masses--------------------------'
echo
for i in `seq 1 $NATOM`
do
  echo "ATOM_MASS[$i]='${ATOM_MASS[$i]}'"
done
# IND_EXCITATION
echo
echo '#-----------------Excitation indicator-----------------------'
echo
for i in `seq 1 $NATOM`
do
  echo "IND_EXCITATION[$i]='${IND_EXCITATION[$i]}'"
done

