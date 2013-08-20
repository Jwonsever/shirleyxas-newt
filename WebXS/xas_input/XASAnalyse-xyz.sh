
# This script XASAnalyze.sh combines spectra from various atoms in various
# xyz-files with atomic alignment as average spectra by type of atom.
# Note that everything is controlled in the Input_Block.in - from the
# xyz-files considered to the atoms considered.
# You can run this script interactively without arguments.
# Since there is a parallel component for running xas_para.x, then you
# may need to request an interactive shell (qsub -I) and submit from there.
# All spectra are collected together in XAS/Spectra-{type}

# davegp, Dec 20, 2011, TMF

. ./Input_Block.in

# spectral details
SIGMA=0.2   # Gaussian broadening
NENER=1000  # Number of energy points
ELOW=-5     # Lower limit for spectrum
EHIGH=30    # Upper limit for spectrum
NOPr=24     # Processors
TEMP=0.0    # Electron temp

if [ -z $CRAY_SITE_LIST_DIR ]; then
  GLOBAL_PARA_PREFIX="$PARA_PREFIX -np $NOPr"
else
  GLOBAL_PARA_PREFIX="$PARA_PREFIX -n $NOPr"
fi
echo $GLOBAL_PARA_PREFIX

ExecDir="$SHIRLEY_ROOT/bin"
xyz2inp="$SHIRLEY_ROOT/scripts/arvid/xyz2inp.sh"
MyDir=`pwd`

REF=atom
CALCGS=GS

ixyz=0
TYPES=''
for xyzfile in $XYZFILES
do

  if [[ -f $xyzfile ]]; then
    echo " working on xyz-file $xyzfile"
  else
    echo " unable to find xyz-file $xyzfile - stopping"
    exit
  fi

  # This defines the name of the directory where calculation results are stored
  # This should be linked to an xyz file
  CALC=`echo $xyzfile | sed 's/\.xyz$//'`

  if [[ ! -d $MyDir/XAS/$CALC ]]; then
    echo directory $MyDir/XAS/$CALC is missing - stopping
    exit
  fi

  ixyz=$((ixyz + 1))
  inpblk[$ixyz]=$MyDir/XAS/$CALC/Input_Block.in
  cp $MyDir/Input_Block.in ${inpblk[$ixyz]}
  $xyz2inp $xyzfile $XYZUNIT $XASELEMENTS >> ${inpblk[$ixyz]} &
  
done
wait
nxyz=$ixyz

echo

TYPES=''
# now loop over excited atoms of this type
for ixyz in `seq 1 $nxyz`
do

  # source the right file
  . ${inpblk[$ixyz]}
  dir=`dirname ${inpblk[$ixyz]}`

  Atoms=""

  #find all excited atoms
  for i in `seq 1 $NAT` ; do
    IA=${ATOM_SYMBOL[$i]}
    if [ ${IND_EXCITATION[$i]} -eq 1 ]; then
      Atoms="$Atoms $i"
    fi
  done

  #shift spectra
  for atom in $Atoms ; do 

    cd $MyDir

    echo "Reference atom calculations from directory:
  XAS/$REF/${ATOM_SYMBOL[$atom]}"

    if [[ ! -f XAS/$REF/${ATOM_SYMBOL[$atom]}/atom.${ATOM_SYMBOL[$atom]}-GS.scf.out ]]; then
      echo "missing atom GS file: XAS/$REF/${ATOM_SYMBOL[$atom]}/atom.${ATOM_SYMBOL[$atom]}-GS.scf.out"
      exit
    fi
    ERefGS0=`grep '^!' XAS/$REF/${ATOM_SYMBOL[$atom]}/atom.${ATOM_SYMBOL[$atom]}-GS.scf.out | awk '{print $5}' | tail -1`

    if [[ ! -f XAS/$REF/${ATOM_SYMBOL[$atom]}/atom.${ATOM_SYMBOL[$atom]}-${CHAPPROX}.scf.out ]]; then
      echo "missing atom ${CHAPPROX} file: XAS/$REF/${ATOM_SYMBOL[$atom]}/atom.${ATOM_SYMBOL[$atom]}-${CHAPPROX}.scf.out"
      exit
    fi
    ERefES0=`grep '^!' XAS/$REF/${ATOM_SYMBOL[$atom]}/atom.${ATOM_SYMBOL[$atom]}-${CHAPPROX}.scf.out | awk '{print $5}' | tail -1`

    SpecDir=$MyDir/XAS/Spectrum-${ATOM_SYMBOL[$atom]}
    if test ! -d $SpecDir ; then
      mkdir $SpecDir
    fi
    cd $SpecDir

    # if this is a new type, create a new Spectra file and zero counter
    n=`echo "$TYPES" | grep -c "${ATOM_SYMBOL[$atom]}"`
    if [[ $n == 0 ]]; then
      :> Spectra
      TYPES="$TYPES ${ATOM_SYMBOL[$atom]}"
      echo TYPES="$TYPES"

      declare "NAVE_${ATOM_SYMBOL[$atom]}"=0
    fi

    atomw=`seq -w $atom $NAT | head -1`
    XASPrefix=$dir/${ATOM_SYMBOL[$atom]}${atom}/$MOLNAME.${ATOM_SYMBOL[$atom]}${atomw}-$CHAPPROX

    #get the specific energies for shift
    if [[ ! -f ${XASPrefix}.scf.out ]]; then
      echo "missing SCF output: ${XASPrefix}.scf.out"
      exit
    fi
    EXCH=`grep '^!' ${XASPrefix}.scf.out | awk '{print $5}' | tail -1`

    if [[ ! -f $dir/$CALCGS/$MOLNAME.scf.out ]]; then
      echo "missing GS output: $dir/$CALCGS/$MOLNAME.scf.out"
      exit
    fi
    EGS=`grep '^!' $dir/$CALCGS/$MOLNAME.scf.out | awk '{print $5}' | tail -1`

    # find the Fermi level (and CBM)
    $GLOBAL_PARA_PREFIX $ExecDir/efermi.x $PARA_POSTFIX $TEMP ${XASPrefix}.xas.$XAS_ARG $NELEC > ${XASPrefix}.xas.${XAS_ARG}-fermi.out

    ELUMO=`tail ${XASPrefix}.xas.${XAS_ARG}-fermi.out | grep 'CBM =' | awk '{print $3}'`

    E=`echo | awk -v exch=$EXCH -v erefes0=$ERefES0 -v egs=$EGS -v erefgs0=$ERefGS0 -v elumo=$ELUMO '{print (exch-erefes0-egs+erefgs0)*13.6056923 - elumo}'`

    echo "i : (XCH - XCH0) - (GS - GS0) - LUMO = Delta"
    echo $atom : $EXCH $ERefES0 - $EGS $ERefGS0 $ELUMO = $E

    # check for previous .xas file and save
    for file in `ls $dir/${ATOM_SYMBOL[$atom]}${atom}/*.xas 2> /dev/null`; do
      mv $file ${file}-raw
    done

    # run xas_para.x
    $GLOBAL_PARA_PREFIX $ExecDir/xas_para.x $PARA_POSTFIX $ELOW $EHIGH $NENER $SIGMA $E $ELUMO ${XASPrefix}.xas.$XAS_ARG 2> /dev/null

    XASFile="`echo $dir | sed 's/^.*\///'`-`basename ${XASPrefix}.xas.$XAS_ARG.xas`"
    mv ${XASPrefix}.xas.$XAS_ARG.xas $XASFile

    paste Spectra $XASFile > $$
    mv $$ Spectra
    declare "NAVE_${ATOM_SYMBOL[$atom]}"=$(( NAVE_${ATOM_SYMBOL[$atom]} + 1 ))

  done

done

# loop over types and generate average spectra
for t in $TYPES; do

  if [[ ! -d $MyDir/XAS/Spectrum-$t ]]; then
    echo "Unable to find Spectrum directory for type $t
$MyDir/XAS/Spectrum-$t"
    exit
  fi
  cd $MyDir/XAS/Spectrum-$t
 
  NAVE=$((NAVE_$t))

  AWKSTR=''
  AVESTR='(0'
  for a in `seq 1 $NAVE`
  do
    b=`echo $a | awk '{print $1*5-3}'`
    AWKSTR="${AWKSTR}, \$$b"
    AVESTR="${AVESTR}+\$$b"
  done
  AVESTR="${AVESTR})/$NAVE"
  #echo "{print \$1, ${AVESTR}${AWKSTR}}"
  awk "{print \$1, ${AVESTR}${AWKSTR}}" Spectra > Spectrum-Ave-$t

  echo output located in 
  echo   $SpecDir/Spectrum-Ave-$t
done

#Grab stats
#~/getStats.py $MyDir >> stats.txt

exit
