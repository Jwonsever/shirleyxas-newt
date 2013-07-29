#!/usr/bin/perl -wT
# CGI wrapper for bash wrapper for scraper.
# hooray for bootstrapping!

use strict;
use warnings;

# include our own modules
use lib './lib';

use Local::Scrapers;
use CGI;

my $query = new CGI;

# extract database name from request
my $target_db = '';
my @matches = grep(/^db$/, $query->param);
my $num_matches = length(@matches);
if ($num_matches != 1) {
  die "Must specify database to scrape exactly once.";
} else {
  $target_db = $query->param('db');
}

# get path to scraper executable for target database
my $scraper = &get_scraper_path($target_db);

# get parameter hash corresponding to target database
my %param_types = %{&get_scraper_params($target_db)};

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

my @cmd = ($scraper);
my $scrubbed = "";
my $scrubbed_field = "";
my $input = "";
# assemble parameters
foreach my $field ($query->param) {
  $input = $query->param($field);
  
  # don't try to scrub the database specifier
  next if $field =~ /^db$/;

  # scrub field
  $scrubbed_field = &untaint_field($field);
  if ($scrubbed_field eq "") {
    die "Scraper received unmatched data. Possible breakin attempt:$!";
  }

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
