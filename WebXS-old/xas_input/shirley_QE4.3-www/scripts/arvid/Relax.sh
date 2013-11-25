#!/bin/bash
#PBS -q nano8
#PBS -l nodes=1:ppn=8
#PBS -l walltime=0:15:00

cd $PBS_O_WORKDIR

. ./Input_Block.in

WORKDIR=./relax
if test ! -d $WORKDIR ; then
mkdir $WORKDIR
fi
cd $WORKDIR

INPUT=$MOLNAME.relax.in
OUTPUT=$MOLNAME.relax.out

cat > $INPUT<<EOF
&control
    calculation='relax'
    prefix='$MOLNAME'
    restart_mode='from_scratch'
    tstress = .true.
    tprnfor = .true.
    pseudo_dir = '$PSEUDO_DIR'
    outdir='./'
    wf_collect = .false.
    disk_io='low' 
    etot_conv_thr = 1.d-4
    forc_conv_thr = 1.d-3
    nstep=100
 /
 &system
        ibrav=$IBRAV, a=$A, b=$B, c=$C
        nat=$NAT, ntyp=$NTYP
        ecutwfc=$ECUT_WFC, ecutrho=$ECUT_RHO
        occupations='smearing',degauss=7.d-4
 /  
 &electrons
    mixing_beta=$ELEC_MIXING_BETA
    electron_maxstep=100
    conv_thr=$ELEC_CONV_THR
    diagonalization='$DIAG' 
    diago_david_ndim=$DIAG_NDIM
 /  
 &ions 
 /  
$ATOMIC_SPECIES
$ATOMIC_POSITIONS
$K_POINTS
EOF

echo Relaxing $MOLNAME

PW="$PARA_PREFIX $BIN_DIR/pw.x $PARA_POSTFIX"
$PW <$INPUT >$OUTPUT

exit
