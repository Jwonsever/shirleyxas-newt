#!/usr/bin/perl -w

use FindBin qw($Bin);
use lib "$FindBin::Bin";
use strict;

use Getopt::Std qw(getopt);
use File::Basename qw(basename);
use Math::Trig;

use Packages::FileFormats qw(GetBGFFileInfo addHeader createBGF createHeaders addBoxToHeader insertHeaderRemark);
use Packages::General qw(FileTester CoM PrintProgress GetTime Rotate);
use Packages::BOX qw(GetBox);
use Packages::ManipAtoms qw(GetMols);
use Packages::REPLICATE qw(ReplicateCell);

sub init;
sub updateResByMol;
sub rotateMols;

my ($replicate, $bgfFile, $saveFile, $doCenter, $bondImages, $rotateOpt);
my ($CENTER, $ATOMS, $BONDS, $HEADERS, $BOX, $tmp, $pStr, $MOLS, $updateResNum);

$|++;
&init;
print "Parsing BGF file $bgfFile...";
($ATOMS, $BONDS, $HEADERS) = GetBGFFileInfo($bgfFile, 1);
$BOX = GetBox($ATOMS, undef, $HEADERS);
print "Done\n"; 
$pStr = "Replicating cell $replicate->{STRING}...";
($ATOMS, $BONDS, $BOX) = ReplicateCell($ATOMS, $BONDS, $BOX, $replicate, $doCenter, $bondImages, $pStr,$updateResNum);
$MOLS = &GetMols($ATOMS, $BONDS) if ($updateResNum or $rotateOpt);
&rotateMols($ATOMS, $BONDS, $MOLS) if ($rotateOpt);
&updateResByMol($ATOMS, $MOLS) if ($updateResNum);
print "Creating BGF file $saveFile...";
&insertHeaderRemark($HEADERS, "REMARK $bgfFile replicated $replicate->{STRING}");
&addBoxToHeader($HEADERS, $BOX);
&addHeader($ATOMS, $HEADERS);
&createBGF($ATOMS, $BONDS, $saveFile);
print "Done\n";

sub updateResByMol {
    my ($atoms, $mols) = @_;
    my ($i);

    for $i (values %{ $atoms }) {
	$i->{RESNUM} = ${ $i->{MOLECULEID} };
    }
}

sub rotateMols {
    my ($atoms, $bonds, $mols) = @_;
    my ($i, $angles, $currMol, $j, $CoM, $pStr, $start, $tot, $count, $strLen);

    $start = time();
    $tot = scalar(keys %{ $mols });
    $pStr = "Randomely rotating molecules...";
    print "${pStr}calculating time remaining\r";
    $count = 1;
    for $i (keys %{ $mols }) {
        $currMol = ();
        for $j (keys %{ $mols->{$i}{MEMBERS} }) {
            $currMol->{$j} = $atoms->{$j};
        }
        @{ $angles } = (2*pi*rand(), 2*pi*3.142*rand(), 2*pi*rand());
        $CoM = CoM($currMol);
        for $j (keys %{ $currMol }) {
            for ("XCOORD", "YCOORD", "ZCOORD") {
                $atoms->{$j}{$_} -= $CoM->{$_};
            }
        }
        Rotate($currMol, $angles, 3);
        for $j (keys %{ $currMol }) {
            for ("XCOORD", "YCOORD", "ZCOORD") {
                $atoms->{$j}{$_} += $CoM->{$_};
            }
        }
        $strLen = PrintProgress($count, $tot, $start, $pStr);
        $count++;
    }
    $tot = GetTime(time() - $start);
    printf "$pStr%-${strLen}s\n", "${tot}s elapsed...Done";
}

sub init {
    my (%OPTS, $rStr);
    getopt('bdsciru',\%OPTS);
    die "usage: $0 -b bgf file -d \"x y z\" replication\n" . 
	"\t-s (save name) -c (center=no) -i (make periodic bonds=yes) -r (randomely rotate=no) -u (update resid=no)\n" 
	if (! exists($OPTS{b}) or ! exists($OPTS{d}));
    print "Initializing...";
    ($bgfFile, $rStr, $saveFile, $doCenter, $bondImages, $rotateOpt, $updateResNum) = 
	($OPTS{b}, $OPTS{d}, $OPTS{s}, $OPTS{c}, $OPTS{i}, $OPTS{r}, $OPTS{u});
    FileTester($bgfFile);
    if ($rStr !~ /(\d+)\s+(\d+)\s+(\d+)/) {
	die "ERROR: Expected integers for x,y and z replication. Got \"$rStr\"\n";
    } else {
	$replicate = (
		      {
			  "X"      => $1,
			  "Y"      => $2,
			  "Z"      => $3,
			  "STRING" => "${1}x${2}x${3}",
		      }
		      );
	die "ERROR: Need at least 1 dimension replication > 1!\n" if ($1 == $2 and $2 == $3 and $3 == 1);
    }

    if (! defined($saveFile)) {
	$saveFile = basename ($bgfFile);
	$saveFile =~ s/\.\w+$//;
	$saveFile .= "_" . $replicate->{STRING} . ".bgf";
    }
    $doCenter = 0 if (! defined($doCenter) or $doCenter !~ /1|yes/i);
    $bondImages = 1 if (! defined($bondImages) or $bondImages !~ /0|no/i);
    $rotateOpt = 0 if (! defined($rotateOpt) or $rotateOpt !~ /1|yes/i);
    $rotateOpt = 1 if ($rotateOpt =~ /1|yes/i);
    $updateResNum = 0 if (! defined($updateResNum) or $updateResNum !~ /1|yes/i);
    $updateResNum = 1 if ($updateResNum =~ /1|yes/i);

    srand(time() ^($$ + ($$ <<15)));
    print "Done\n";
}
