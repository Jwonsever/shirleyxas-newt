#!/bin/tcsh
#!/bin/csh

# Load Global Variables
echo `pwd`
set scriptDir = `pwd`
/bin/bash $scriptDir/../../GlobalValues.in

if ($#argv < 1) then
  echo "usage: $0 cif_file|xyz_file [mol_prefix] [cell_size=10x10x10x90x90x90] [MD_temp=298K] [MD_press=1Bar] [nodes=6] [ppn=24] [cp2k_template_file] [pbs_template_file]"
  exit(1)
endif

set cif_file = $1
if !(-e $cif_file) then
  echo "ERROR: Cannot locate file $1"
  exit(1)
endif
set prefix = `basename $cif_file`
set prefix = $prefix:r
if ($#argv > 1) set prefix = $2

set cellsize = (10 10 10 90 90 90)
if ($#argv > 2) set cellsize = `echo $3 | sed 's/[a-z]*/ /gi'`
set cell = ($cellsize)

if ($#cell != 6) then
  echo $#cell
  echo "ERROR: Expected 'a b c alpha beta gamma' for cell. Got $cellsize"
  exit(1)
endif

set temp = 298
if ($#argv > 3) set temp  = `echo $4 | sed 's/[a-zA-Z]*//gi'`

set pressure = 1.0
if ($#argv > 4) set pressure  = `echo $5 | sed 's/[a-zA-Z]*//gi'`

set nodes = 6
if ($#argv > 5) set nodes = $5

set ppn = 24
if ($#argv > 6) set ppn = $6

set cp2k_template_file = ~/cp2k/webcp2k/cp2k.template.in
if ($#argv > 7) set cp2k_template_file = $7

set pbs_template_file = ~/cp2k/webcp2k/template.cp2k.pbs
if ($#argv > 8) set pbs_template_file = $8

set repstr = "1x1x1"
set tprocs=`echo "$nodes * $ppn" | bc -l`

set ext = $cif_file:e
if ($ext == "cif") then 
  set cell = `grep -e cell_ $cif_file | awk '{print $2}' | sed 's/([^)]*)//g'`

#convert to xyz coords
  echo "copy ${cif_file} .xyz" | ~/gdis-0.90/gdis >& /dev/null
  mv $cif_file.xyz $prefix.$repstr.xyz
else
  cp $cif_file $prefix.$repstr.xyz
endif

cat ${prefix}.${repstr}.xyz | awk '{if(NR>2) { printf "%-4s %6.5f %6.5f %6.5f\n",$1,$2,$3,$4} }' > ${prefix}.${repstr}.cp2k.xyz

cp $cp2k_template_file ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/LA_HERE/$cell[1]/" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/LB_HERE/$cell[2]/" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/LC_HERE/$cell[3]/" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/ALPHA_HERE/$cell[4]/" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/BETA_HERE/$cell[5]/" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/GAMMA_HERE/$cell[6]/" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/TEMP_HERE/$temp/" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/PRESSURE_HERE/$pressure/" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/PREFIX_HERE/${prefix}.${repstr}/" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/CP2KHOME_HERE/${CODE_BASE_DIR}${CP2K_LOC}/" ${prefix}.${repstr}.${temp}K.cp2k.in

set basis_set_file = "~/cp2k/webcp2k/cp2k/tests/QS/GTH_BASIS_SETS ~/cp2k/webcp2k/cp2k/tests/QS/BASIS_MOLOPT"
set pseudo_file = "~/cp2k/webcp2k/cp2k/tests/QS/GTH_POTENTIALS ~/cp2k/webcp2k/cp2k/tests/QS/POTENTIAL"

echo "" > ${prefix}.kind.dat
rm -fr __tmp.dat
foreach i (`cat ${prefix}.${repstr}.cp2k.xyz | awk '{print $1}'`)
  if (-e ${prefix}.kind.${i}.dat) continue
  set list = (`grep -c ${i} $basis_set_file | grep GTH | sed 's/.*://'`)
  if ($#list == 0) then
    echo "ERROR: Cannot find DZVP-GTH entry for $i in $basis_set_file"
    exit(1)
  endif
  set list = (`grep -c ${i} $pseudo_file | grep GTH | sed 's/.*://'`)
  if ($#list == 0) then
    echo "ERROR: Cannot finf GTH-PBE entry for $i in $pseudo_file"
    exit(1)
  endif
  set pseudo_str = `grep "${i} GTH-PBE-" $pseudo_file | head -1 | awk '{print $2}'`
  cat > ${prefix}.kind.${i}.dat <<DATA;

  &KIND $i
   BASIS_SET DZVP-GTH
   POTENTIAL $pseudo_str
  &END KIND
DATA

  cat ${prefix}.kind.dat ${prefix}.kind.${i}.dat > __tmp.dat
  mv __tmp.dat ${prefix}.kind.dat
end
sed -i "/KIND_HERE/r ${prefix}.kind.dat" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i '/KIND_HERE/d' ${prefix}.${repstr}.${temp}K.cp2k.in


cp $pbs_template_file ${prefix}.${repstr}.${temp}K.cp2k.pbs
sed -i "s/nodes_here/$nodes/" ${prefix}.${repstr}.${temp}K.cp2k.pbs
sed -i "s/ppn_here/$ppn/" ${prefix}.${repstr}.${temp}K.cp2k.pbs
sed -i "s/procs_here/$tprocs/" ${prefix}.${repstr}.${temp}K.cp2k.pbs
sed -i "s/prefix_here/${prefix}/" ${prefix}.${repstr}.${temp}K.cp2k.pbs
sed -i "s/fprefix_here/${prefix}.${repstr}.${temp}K/" ${prefix}.${repstr}.${temp}K.cp2k.pbs
sed -i "s/cell_p_here/$repstr/" ${prefix}.${repstr}.${temp}K.cp2k.pbs
rm -fr __tmp.dat ${prefix}.kind*.dat 

exit:
exit(0)

error:
echo "ERROR occurred"
exit(1)
