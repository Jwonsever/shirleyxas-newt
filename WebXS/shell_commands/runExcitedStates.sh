#!/bin/bash

#Run Excited States After Completion of XAS-ANALYZE
#GZIP STATE FILES, Remove Other Files
#James Wonsever

SHELL_ROOT_SCRIPTS="/project/projectdirs/mftheory/www/james-xs/WebXS/shell_commands/"

WHERE_AM_I=`pwd`
JOB_NAME=`basename ${WHERE_AM_I}`

#get Scripts
/usr/bin/python ${SHELL_ROOT_SCRIPTS}findNotableStates.py $WHERE_AM_I $JOB_NAME > states.csv

#write pp.x scripts, and aprun all  
ppExe="/project/projectdirs/mftheory/hopper/shirley_QE4.3-intel/bin/pp.x"
function plot {

cat > PPin/$1.in <<EOF
&INPUTPP
  prefix="${2}"
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

aprun -n 24 ${ppExe} < PPin/$1.in >> PPout/$1.out
}

while IFS=, read atom model state ev str
do
   cd $WHERE_AM_I

   model=`echo "$model - 1" | bc`
   dirext="./XAS/"$JOB_NAME"_"$model"/"$atom
   cd $dirext

   mkdir "PPout"
   mkdir "PPin"

   prefix=$JOB_NAME"."$atom"-XCH"
 
   plot $state $prefix
done < states.csv

#Gzip all cube files
find ./ -type f -name "*.cube" -exec tar -cvzf '{}'.tar.gz {} \;
#Remove all excess Data
#${SHELL_ROOT_SCRIPTS}wipeExcessFiles.sh `pwd` 

exit