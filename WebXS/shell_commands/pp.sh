#!/bin/bash -l
#Makes PP.in then runs PP and PPlot.  Runs jobs on hopper debug queue.
#Version .717

dir=$1
#ls to find this???
prefix=$2
#Same as the number of stick files.
procsUsed=$3
PPP=$4
lband=$5
hband=$6

#Should be a /pp directory
cd $dir
#make output file
mkdir "PPout"
mkdir "PPin"

ppPBS="#!/bin/bash\n"
ppPBS+="#PBS -q debug\n" #debug is 30min max
ppPBS+="#PBS -l walltime=00:30:00\n"
ppPBS+="#PBS -V\n"
ppPBS+="#PBS -e pp.err\n"
ppPBS+="#PBS -o pp.out\n"
ppPBS+="#PBS -N PP:${lband}-${prefix}\n"
ppPBS+="#PBS -l mppnppn=${PPP}\n"
ppPBS+="#PBS -l mppwidth=${procsUsed}\n\n"

ppPBS+="cd ${dir}\n"
ppPBS+="export NO_STOP_MESSAGE=1\n\n"

ppExe="/project/projectdirs/mftheory/hopper/shirley_QE4.3-intel/bin/pp.x"

#prepending?
#mv myfile tmp
#cat myheader tmp > myfile
#rm tmp

function plot {

cat > PPin/$1.in <<EOF
&INPUTPP
  prefix="${prefix}"
  outdir="."
  filplot="state.$1.dat"
  plot_num=7
  kpoint=1
  kband=$1
  spin_component=0
  lsign=.true.
/
&PLOT
  iflag=3
  output_format=6
  fileout="state.$1.cube"
/
EOF

ppPBS+="aprun -n 24 ${ppExe} < PPin/$1.in >> PPout/$1.out\n\n"
}

for i in `seq $lband $hband`
do
    plot $i
done

echo -e ${ppPBS} > ./pp.qscript

## Submit pp
pp_id=`/usr/common/nsg/bin/qsub pp.qscript `

#somehow let app know the job is finished