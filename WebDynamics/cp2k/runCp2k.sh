#!/bin/sh

#TODO switch from jwonsever home paths to web paths

# Load Global Variables
scriptDir=`dirname $0`
. $scriptDir/../../GlobalValues.in

CK2K_PATH=$CODE_BASE_DIR$CP2K_LOC

if [[ $# < 1 ]]; then
  echo "usage: $0 cif_file|xyz_file [mol_prefix] [cell_size=10x10x10x90x90x90] [MD_temp=298K] [MD_press=1Bar] [nodes=6] [ppn=24] [cp2k_template_file] [pbs_template_file]"
  exit 1
fi


outputPath=`dirname $1`
cd $outputPath
    
cif_file=$1
if [ ! -f $cif_file ]; then
  echo "ERROR: Cannot locate file $1"
  exit 1
fi

prefix=`basename $cif_file`
prefix=$prefix:r
if [[ $# > 1 ]]; then
    prefix=$2
fi

cellsize=(10 10 10 90 90 90)
if [[ $# > 2 ]]; then
    cellsize=`echo $3 | sed 's/[a-zA-Z]*/ /g'`
fi

cell=($cellsize)

if [ ${#cell[@]} != 6 ]; then
  echo ${#cell[@]}
  echo "ERROR: Expected 'a b c alpha beta gamma' for cell. Got $cellsize"
  exit 1
fi

temp=298
if [[ $# > 3 ]]; then 
    temp=`echo $4 | sed 's/[a-zA-Z]//gi'`
fi
echo $temp

pressure=1.0
if [[ $# > 4 ]]; then
    pressure=`echo $5 | sed 's/[a-zA-Z]//gi'`
fi
echo $pressure

nodes=6
if [[ $# > 5 ]]; then 
    nodes=$5
fi

ppn=24
if [[ $# > 6 ]]; then
    ppn=$6
fi

cp2k_template_file=$CP2K_PATH/cp2k.template.in
if [[ $# > 7 ]]; then 
    cp2k_template_file=$7
fi

pbs_template_file=$CP2K_PATH/template.cp2k.pbs
if [[ $# > 8 ]]; then 
    pbs_template_file=$8
fi

repstr="1x1x1"

tprocsflt=`echo "$nodes * $ppn" | bc -l`
tprocs="${tprocs##*.}"
echo "Total Processors: "tprocs

filename=$(basename "$cif_file")
ext="${filename##*.}"
echo "FileExt: "ext

if [ $ext == "cif" ]; then 
  cell=`grep -e cell_ $cif_file | awk '{print $2}' | sed 's/([^)]*)//g'`

#convert to xyz coords
  echo "copy ${cif_file} .xyz" | /global/u2/w/wonsever/gdis-0.90/gdis 2>&1 > /dev/null
  mv $cif_file.xyz $prefix.$repstr.xyz
else
  cp $cif_file $prefix.$repstr.xyz
fi

cat ${prefix}.${repstr}.xyz | awk '{if(NR>2) { printf "%-4s %6.5f %6.5f %6.5f\n",$1,$2,$3,$4} }' > ${prefix}.${repstr}.cp2k.xyz

cp $cp2k_template_file ${prefix}.${repstr}.${temp}K.cp2k.in

sed -i "s/LA_HERE/$cell[1]/g" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/LB_HERE/$cell[2]/g" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/LC_HERE/$cell[3]/g" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/ALPHA_HERE/$cell[4]/g" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/BETA_HERE/$cell[5]/g" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/GAMMA_HERE/$cell[6]/g" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/TEMP_HERE/$temp/g" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/PRESSURE_HERE/$pressure/g" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/PREFIX_HERE/${prefix}.${repstr}/g" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/CP2KHOME_HERE/${CP2K_PATH}/g" ${prefix}.${repstr}.${temp}K.cp2k.in

basis_set_file="$CP2K_PATH/cp2k/tests/QS/GTH_BASIS_SETS $CP2K_PATH/cp2k/tests/QS/BASIS_MOLOPT"
pseudo_file="$CP2K_PATH/cp2k/tests/QS/GTH_POTENTIALS $CP2K_PATH/cp2k/tests/QS/POTENTIAL"

echo "" > ${prefix}.kind.dat
rm -fr __tmp.dat

atomList=( `cat ${prefix}.${repstr}.cp2k.xyz | awk '{print $1}'` ) 
for i in "${atomList[@]}"
do
  
  echo $i

  if [ -e ${prefix}.kind.${i}.dat ]; then
      continue
  fi

  list=(`grep -c ${i} $basis_set_file | grep GTH | sed 's/.*://'`)
  if [ ${#list[@]} == 0 ]; then
    echo "ERROR: Cannot find DZVP-GTH entry for $i in $basis_set_file"
    exit 1
  fi
  list=(`grep -c ${i} $pseudo_file | grep GTH | sed 's/.*://'`)
  if [ ${#list[@]} == 0 ]; then
    echo "ERROR: Cannot finf GTH-PBE entry for $i in $pseudo_file"
    exit 1 
  fi
  pseudo_str=`grep "${i} GTH-PBE-" $pseudo_file | head -1 | awk '{print $2}'`
  cat > ${prefix}.kind.${i}.dat <<DATA;

  &KIND $i
   BASIS_SET DZVP-GTH
   POTENTIAL $pseudo_str
  &END KIND
DATA

  cat ${prefix}.kind.dat ${prefix}.kind.${i}.dat > __tmp.dat
  mv __tmp.dat ${prefix}.kind.dat
done

sed -i "/KIND_HERE/r ${prefix}.kind.dat" ${prefix}.${repstr}.${temp}K.cp2k.in

sed -i "/KIND_HERE/d" ${prefix}.${repstr}.${temp}K.cp2k.in


cp $pbs_template_file ${prefix}.${repstr}.${temp}K.cp2k.pbs
sed -i "s/nodes_here/$nodes/g" ${prefix}.${repstr}.${temp}K.cp2k.pbs
sed -i "s/ppn_here/$ppn/g" ${prefix}.${repstr}.${temp}K.cp2k.pbs
sed -i "s/procs_here/$tprocs/g" ${prefix}.${repstr}.${temp}K.cp2k.pbs
#todo: wallhours from tool, smartly calculated
sed -i "s/wallhours_here/04/g" ${prefix}.${repstr}.${temp}K.cp2k.pbs
#also todo, selective number of snapshots
sed -i "s/fprefix_here/${prefix}.${repstr}.${temp}K/g" ${prefix}.${repstr}.${temp}K.cp2k.pbs
sed -i "s/prefix_here/${prefix}/g" ${prefix}.${repstr}.${temp}K.cp2k.pbs
sed -i "s/cell_p_here/$repstr/g" ${prefix}.${repstr}.${temp}K.cp2k.pbs
rm -fr __tmp.dat ${prefix}.kind*.dat 

#Start the calculation
qsub *.pbs