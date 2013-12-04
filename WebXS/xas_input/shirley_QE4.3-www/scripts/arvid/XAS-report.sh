#!/bin/bash

function duration {
  dir=$1
  step=$2
  if [ -f $dir/*${step}in ]; then
    firstin=`ls -rt $dir/*${step}in | tail -1`
  else
    echo $dir $step input missing
    return -1
  fi
  shopt -s nullglob
  outs=($dir/*${step}out*)
  if (( ${#outs[@]} > 0 )); then
    lastout=`ls -rt $dir/*${step}out* | tail -1`
  else
    echo $dir $step output missing
    return -1
  fi
  seconds=`echo $(( $(date -r $lastout +%s) - $(date -r $firstin +%s) ))`
  minutes=`echo "scale=2; $seconds / 60" | bc`
  echo $dir $step duration: $minutes m
  return $seconds
}

echo XAS report
echo ==========

if [ ! -d XAS ]; then
  echo Error: No XAS directory
  exit
fi

# Check for ref calculations
if [ -d XAS/atom ]; then
  dir=XAS/atom
  echo atom directory present
  for atom in $dir/*; do
    if [ -d $atom ]; then
      echo $atom

      duration $atom GS.scf.
      duration $atom XCH.scf.

    fi
  done
else
  echo atom directory missing - qsub ref
fi
for xyz in `ls *.xyz`; do
  xyz=${xyz%.xyz}
  dir=XAS/$xyz/GS
  if [ ! -d $dir ]; then
    echo missing $dir : qsub ref
  else
    duration $dir .scf.
  fi
done

echo ==========

tottime=0
for xyz in `ls *.xyz`; do
  xyz=${xyz%.xyz}
  for dir in XAS/$xyz/*; do
    if [[ $dir =~ GS$ ]]; then continue; fi
    if [ -d $dir ]; then
      snaptime=0
      for step in scf nscf basis ham xas; do
        duration $dir .$step.
        snaptime=$(( $? + snaptime ))
      done
      echo $dir time: `echo "scale=2; $snaptime / 60" | bc` m
      tottime=$(( snaptime + tottime ))
    fi
  done
done
echo total xas time: `echo "scale=2; $tottime / 60" | bc`  m
echo ==========
echo XAS report - complete

