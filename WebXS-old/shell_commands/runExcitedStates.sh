#!/bin/bash

#Run Excited States After Completion of XAS-ANALYZE
#BZIP2 STATE FILES, Remove Other Files
#James Wonsever

# Load Global Variables
. $scriptDir/../../GlobalValues.in

SHELL_ROOT_SCRIPTS=$CODE_BASE_DIR/$CODE_LOC/$SERVER_SCRIPTS/

WHERE_AM_I=`pwd`
JOB_NAME=`basename ${WHERE_AM_I}`

#get Scripts
/usr/bin/python ${SHELL_ROOT_SCRIPTS}findNotableStates.py $WHERE_AM_I $JOB_NAME > states.csv

#write pp.x scripts, and aprun all  
ppExe="${SHIRLEY_ROOT}/bin/pp.x"
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

aprun -n 24 ${ppExe} < PPin/$1.in >> PPout/$1.out &
}

while IFS=, read atom model state ev str
do
   cd $WHERE_AM_I

   model=`echo "$model - 1" | bc`

   #Because of the fname difference between [ex. c002 and c2]
   shortatom=`echo "${atom}" | sed 's/\(^[a-zA-Z]*\)[0]*/\1/'`
   dirext="./XAS/"$JOB_NAME"_"$model"/"$shortatom
   cd $dirext

   mkdir "PPout"
   mkdir "PPin"

   prefix=$JOB_NAME"."$atom"-XCH"
 
   plot $state $prefix
done < states.csv

#bzip all cube files
find ./ -type f -name "*.cube" -exec bzip2 -k9 {} \;

wait

#Archive Everything
#todo

#Remove all excess Data  (todo, needs to be AFTER all the other jobs complete.)
#${SHELL_ROOT_SCRIPTS}wipeExcessFiles.sh `pwd` 

exit