#!/usr/bin/perl -wT
# CGI wrapper for bash wrapper for scraper.
# hooray for bootstrapping!

use strict;
use warnings;

use CGI;
my $query = new CGI;

# trims whitespace
my %regexes = (
  'num' => qr/^\s*(\d*)\s*$/,
  'alpha_num' => qr/^\s*([a-zA-Z\d]*?)\s*$/,
  'alpha_space' => qr/^\s*([a-zA-Z\s]*?)\s*$/,
  'alpha_num_space' => qr/^\s*([a-zA-Z\d\s]*?)\s*$/,
  'alpha_num_space_paren' => qr/^\s*([a-zA-Z\d\s\(\)]*?)\s*$/,
  'alpha_space_paren_dash' => qr/^\s*([a-zA-Z\s\(\)-]*?)\s*$/
);

my %param_types = (
  'composition' => $regexes{'alpha_num_space'},
  'num_elements' => $regexes{'num'},
  'struct_fmla' => $regexes{'alpha_num_space_paren'},
  'chem_name' => $regexes{'alpha_space'},
  'mineral_name' => $regexes{'alpha_space_paren_dash'},
  'mineral_grp' => $regexes{'alpha_space'},
  'anx_fmla' => $regexes{'alpha_num'},
  'ab_fmla' => $regexes{'alpha_num'},
  'num_fmla_units' => $regexes{'num'}
);

sub untaint
{
  my($field, $input) = ($_[0], $_[1]);
  my $re = $param_types{$field};
  if ($input =~ $re) {
    $input = $1;
  } else {
    $input = "";
  }
  return $input;
}

# construct parameter scrubber from expected parameter hash
my $builder = "(";
foreach my $expected_param (keys %param_types) {
  $builder .= $expected_param . "|";
}
# remove trailing extra |
$builder = substr($builder, 0, length($builder) - 1);
$builder .= ")";
my $expected_params = qr/$builder/;

sub untaint_field
{
  my $field = $_[0];
  if ($field =~ $expected_params) {
    $field = $1;
  } else {
    $field = "";
  }
  return $field;
}

# TODO: also scrub the field. We already have a list of acceptable ones,
# so won't be too much trouble.
# Can consruct an alternation (|) from the list of expectedi inputs.
# It has to match one, and then capture it.
# This will make Taint happy.
my @cmd = ("./scrape.sh");
my $scrubbed = "";
my $scrubbed_field = "";
my $input = "";
# assemble scrubbedeters
foreach my $field ($query->param) {
  $input = $query->param($field);

  # scrub field
  $scrubbed_field = &untaint_field($field);
  if ($scrubbed_field eq "") {
    die "Scraper received unmatched data. Possible breakin attempt:$!";
  } # else continue

  # scrub input
  $scrubbed = &untaint($field, $input);
  if ($scrubbed eq "") {
    die "Scraper received unmatched data. Possible breakin attempt:$!";
  } else {
    push(@cmd, "--$scrubbed_field");
    push(@cmd, "$scrubbed");
  }
}

$ENV{PATH} = "/bin:/usr/bin";
delete @ENV{qw(IFS CDPATH ENV BASH_ENV)};   # Make %ENV safer
print "Content-type: application/json\n\n";
foreach my $c (@cmd) {
  print "$c\n";
}
system(@cmd);
print "[]\n";
