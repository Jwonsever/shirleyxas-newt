#!/bin/bash
# Load user variables
. ./Input_Block.in

# Location of executables and script libraries
BibDir="$SHIRLEY_ROOT/scripts/arvid/Bibliothek"
export BibDir
ExecDir="$SHIRLEY_ROOT/bin"

# Calculation-specific file info
MyDir=`pwd`
export NodePool="${MyDir}/Nodes"
ScriptName=`echo "$0" | sed -e 's/\.\///g'`

# useful script shortcuts
ReVa="${BibDir}/ResetVariables.sh"
VP="${BibDir}/VarPen.sh" #Executable to change the file TMP_INPUT.in
Re="${BibDir}/Reverend.sh"
GeNo="${BibDir}/GetNode.sh"
xyz2inp="${BibDir}/../xyz2inp.sh"

#end of declarations

if [[ ! -d $MyDir/XAS ]]; then
echo "missing directory:"
echo "$MyDir/XAS"
exit
fi

# number of atoms to be calculated
NAT_CALC=0

for xyzfile in $XYZFILES
do

  if [[ -f $xyzfile ]]; then
    echo " working on xyz-file $xyzfile"
  else
    echo " unable to find xz-file $xyzfile - skipping"
    continue
  fi

  # This defines the name of the directory where calculation results are stored
  # This should be linked to an xyz file
  CALC=`echo $xyzfile | sed 's/\.xyz$//'`

  if [[ ! -d $MyDir/XAS/$CALC ]]; then
    echo "missing directory:"
    echo "$MyDir/XAS/$CALC"
    continue
  fi

  # Make a copy of Input_Block.in in the working directory
  inpblk=$MyDir/Input_Block.in.${CALC}
  cp $MyDir/Input_Block.in $inpblk
  echo " converting xyz to input..."
  $xyz2inp $xyzfile $XYZUNIT $XASELEMENTS >> $inpblk
  echo " ... done"
  . $inpblk

  # loop over atoms
  for i in `seq 1 $NAT` ; do

    if [ ${IND_EXCITATION[$i]} -eq 1 ] ; then

      NAT_CALC=$(( NAT_CALC + 1 ))

      dir=$MyDir/XAS/$CALC/${ATOM_SYMBOL[$i]}${i}
      if test ! -d $dir ; then
        echo "missing directory:"
        echo "$dir"
        continue
      fi

      CATOM=`seq -w $i $NAT | head -1`

      # Check the calc
      echo -n " checking atom ${ATOM_SYMBOL[$i]}$i"
      #${BibDir}/DoXAS.sh $dir $i ${ATOM_SYMBOL[$i]} $NodePool &
      #Check what actually needs to be done and define JOB
      TMP_MOLNAME=${MOLNAME}.${ATOM_SYMBOL[$i]}${CATOM}-${CHAPPROX}
      SCFOUT=$dir/$TMP_MOLNAME.scf.out
      NSCFOUT=$dir/$TMP_MOLNAME.nscf.out
      BASISOUT=$dir/$TMP_MOLNAME.basis.out
      HAMOUT=$dir/$TMP_MOLNAME.ham.out
      XASOUT=$dir/$TMP_MOLNAME.xas.out

      DONE=''
      if [ ! -f $XASOUT ] ||  (! grep -q 'end shirley_xas' $XASOUT)
      then
        echo "
  xas missing"
        DONE='1'
      fi
      if [ ! -f $HAMOUT ] || (! grep -q 'fft_scatter' $HAMOUT)
      then
        echo "
  ham missing"
        DONE='2'
      fi
      if [ ! -f $BASISOUT ] || (! grep -q 'fft_scatter' $BASISOUT)
      then
        echo "
  basis missing"
        DONE='3'
      fi
      if [ ! -f $NSCFOUT ] || (! grep -q 'fft_scatter' $NSCFOUT)
      then
        echo "
  nscf missing"
        DONE='4'
      fi
      if [ ! -f $SCFOUT ] || (! grep -q 'convergence has been achieved' $SCFOUT)
      then
        echo "
  scf missing"
        DONE='5'
      fi
      if [ -z $DONE ]
      then
        echo "  complete"
        NAT_CALC=$(( NAT_CALC - 1))
      fi

    fi

  done
  rm $inpblk
done
echo "remaining atoms to compute: $NAT_CALC"
exit
