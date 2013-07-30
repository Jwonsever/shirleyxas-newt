#!/bin/tcsh
#!/bin/csh

if ($#argv < 1) then
  echo "usage: $0 cif_file|bgf_file [mol_prefix] [cell_replication=2x2x2] [MD_temp=298K] [nodes=6] [ppn=8] [cp2k_template_file] [pbs_template_file]"
  exit(1)
endif

set cif_file = $1
if !(-e $cif_file) then
  echo "ERROR: Cannot locate CIF file $1"
  exit(1)
endif
set prefix = `basename $cif_file`
set prefix = $prefix:r
if ($#argv > 1) set prefix = $2

set replication = "2 2 2"
if ($#argv > 2) set replication = `echo $3 | sed 's/[a-z]*/ /gi'`
set rarray = ($replication)
if ($#rarray != 3) then
  echo $#rarray
  echo "ERROR: Expected 'i j k' for replication. Got $replication"
  exit(1)
endif
set repstr = `echo $replication | sed 's/ /x/g'`

set temp = 298
if ($#argv > 3) set temp  = `echo $4 | sed 's/[a-z]*//gi'`

set nodes = 6
if ($#argv > 4) set nodes = $5

set ppn = 8
if ($#argv > 5) set ppn = $6

set cp2k_template_file = ~cp2k/webcp2k/cp2k.template.in
if ($#argv > 6) set cp2k_template_file = $7

set pbs_template_file = ~cp2k/webcp2k/template.cp2k.pbs
if ($#argv > 7) set pbs_template_file = $8

set ext = $cif_file:e
if ($ext == "cif") then 
  csh -f ~tpascal/scripts/cif2bgf.csh $cif_file ${prefix}.bgf > /dev/null || goto error
  if (`echo $rarray | awk '{if ($1 == 1 && $2 == 1 && $3 == 1) { print 0; }else { print 1;} }'`) then
    ~tpascal/scripts/replicate.pl -b ${prefix}.bgf -d "$replication" -s ${prefix}.${repstr}.bgf >& /dev/null || goto error
  else
    cp ${prefix}.bgf ${prefix}.${repstr}.bgf
  endif
  set bgf_file = ${prefix}.${repstr}.bgf
else
  set bgf_file = $cif_file
endif

babel -ibgf $bgf_file -oxyz ${prefix}.${repstr}.xyz >& /dev/null || goto error
set cell = (`grep "^CRYSTX" $bgf_file | awk '{print $2,$3,$4,$5,$6,$7}'`)
sed -i "s/^${bgf_file}/${prefix} ${repstr} CELL: $cell/" ${prefix}.${repstr}.xyz
cat ${prefix}.${repstr}.xyz | awk '{if(NR>2) { printf "%-4s %6.5f %6.5f %6.5f\n",$1,$2,$3,$4} }' > ${prefix}.${repstr}.cp2k.xyz
cp $cp2k_template_file ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/LA_HERE/$cell[1]/" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/LB_HERE/$cell[2]/" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/LC_HERE/$cell[3]/" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/ALPHA_HERE/$cell[4]/" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/BETA_HERE/$cell[5]/" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/GAMMA_HERE/$cell[6]/" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/TEMP_HERE/$temp/" ${prefix}.${repstr}.${temp}K.cp2k.in
sed -i "s/PREFIX_HERE/${prefix}.${repstr}/" ${prefix}.${repstr}.${temp}K.cp2k.in

set basis_set_file = "~tpascal/codes/cp2k/trunk/cp2k/tests/QS/GTH_BASIS_SETS ~tpascal/codes/cp2k/trunk/cp2k/tests/QS/BASIS_MOLOPT"
set pseudo_file = "~tpascal/codes/cp2k/trunk/cp2k/tests/QS/GTH_POTENTIALS ~tpascal/codes/cp2k/trunk/cp2k/tests/QS/POTENTIAL"
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

cp $pbs_template_file ${prefix}.${repstr}.${temp}K.cp2k.shirley.pbs
sed -i "s/nodes_here/$nodes/" ${prefix}.${repstr}.${temp}K.cp2k.shirley.pbs
sed -i "s/ppn_here/$ppn/" ${prefix}.${repstr}.${temp}K.cp2k.shirley.pbs
sed -i "s/rtemp_here/$temp/" ${prefix}.${repstr}.${temp}K.cp2k.shirley.pbs
sed -i "s/fprefix_here/$prefix/" ${prefix}.${repstr}.${temp}K.cp2k.shirley.pbs
sed -i "s/cell_p_here/$repstr/" ${prefix}.${repstr}.${temp}K.cp2k.shirley.pbs
sed -i "s/c_a_here/$cell[1]/" ${prefix}.${repstr}.${temp}K.cp2k.shirley.pbs
sed -i "s/c_b_here/$cell[2]/" ${prefix}.${repstr}.${temp}K.cp2k.shirley.pbs
sed -i "s/c_c_here/$cell[3]/" ${prefix}.${repstr}.${temp}K.cp2k.shirley.pbs
sed -i "s/c_alpha_here/$cell[4]/" ${prefix}.${repstr}.${temp}K.cp2k.shirley.pbs
sed -i "s/c_beta_here/$cell[5]/" ${prefix}.${repstr}.${temp}K.cp2k.shirley.pbs
sed -i "s/c_gamma_here/$cell[6]/" ${prefix}.${repstr}.${temp}K.cp2k.shirley.pbs
rm -fr __tmp.dat ${prefix}.kind*.dat 

exit:
exit(0)

error:
echo "ERROR occurred"
exit(1)
