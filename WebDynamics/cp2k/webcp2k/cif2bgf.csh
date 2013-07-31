#!/bin/tcsh
#!/bin/csh

if ($#argv == 0) then
  echo "usage: $0 cif_file (bgf_file) [use_babel_conections=no] (gdis_loc=~tpascal/codes/bin/gdis_cmdline)"
  exit(1)
endif

set cif_file = $1
if !(-e $cif_file) then
 echo "ERROR: Cannot access CIF file $cif_file"
  exit(1)
endif

set bgf_file = `basename $cif_file`
set bgf_file = $bgf_file:r
set bgf_file = "${bgf_file}.bgf"
if ($#argv > 1) set bgf_file = $2

set use_babel_conect = 0
if ($#argv > 2) set use_babel_conect = $3

set tmp_bgf = __tmp.bgf
if ($use_babel_conect) set tmp_bgf = __temp.new.bgf

set gdis_cmd = ~tpascal/codes/bin/gdis_cmdline
if ($#argv > 3) set gdis_cmd = "$4"

cat > __gdis_script <<DATA;
copy $cif_file __tmp.pdb
copy $cif_file __tmp.bgf
DATA

$gdis_cmd < __gdis_script > /dev/null || goto error

if !(-e __tmp.pdb) then
  echo "ERROR: Cannot execute gdis cmd $gdis_cmd"
  exit(1)
endif
if !(-e __tmp.bgf) then
  echo "ERROR: Cannot execute gdis cmd $gdis_cmd"
  exit(1)
endif

set cell = (`grep CRYST __tmp.pdb | awk '{print $2,$3,$4,$5,$6,$7}'`)
echo $cell
sed -i '/CONECT.*$/,$d' __tmp.pdb

~tpascal/codes/bin/babel -ipdb __tmp.pdb -obgf __temp.new.bgf > /dev/null || goto error
~tpascal/scripts/fixBGFfftype.pl -b __temp.new.bgf -s __temp.new.bgf -f 0 > /dev/null || goto error
set con_start = `grep -n "^FORMAT CONECT" __temp.new.bgf | head -1 | sed 's/:.*$//'`
head -${con_start} __temp.new.bgf > __temp.new.nobonds.bgf
set con_start = `grep -n "^CONECT" $tmp_bgf | head -1 | sed 's/:.*$//'`
set tsize = `wc -l $tmp_bgf | awk '{print $1}'`
set tot = `echo $tsize $con_start | awk '{print $1-$2+1}'`
tail -${tot} $tmp_bgf > __bonds.dat
cat __temp.new.nobonds.bgf __bonds.dat > $bgf_file
~tpascal/scripts/getBGFAtoms.pl -b $bgf_file -s $bgf_file -o "index>0" > /dev/null || goto error
cat > __cell.dat <<DATA;
PERIOD 111
AXES   ZYX
SGNAME P 1                  1    1
DATA

sed -i '/^FORCEFIELD/r __cell.dat' $bgf_file

cat > __cell.dat <<DATA;
CELLS    -1    1   -1    1   -1    1
DATA

sed -i '/^CRYSTX/r __cell.dat' $bgf_file

#/home/tpascal/scripts/addBoxToBGF.pl $bgf_file /home/tpascal/ff/DREIDING2.21.ff $bgf_file > /dev/null || goto error
#/home/tpascal/scripts/updateBGFBox.pl -b $bgf_file -s $bgf_file -c "$cell" > /dev/null || goto error
rm -fr __temp.new.nobonds.bgf __tmp.bgf __bonds.dat __temp.new.bgf __tmp.pdb  __gdis_script gmon.out __cell.dat 

exit:
echo "All taksks completed"
exit(0)

error:
echo "Error occurred"
exit(1)
