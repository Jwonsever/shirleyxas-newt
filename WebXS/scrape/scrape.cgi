#!/usr/bin/perl -wT
# CGI wrapper for bash wrapper for scraper.
# hooray for bootstrapping!

use strict;
use warnings;
#use diagnostics;

# include our own modules
use lib './lib';

use Local::Scrapers qw(scrapers get_scraper_params);
use CGI;

my $query = new CGI;

my $executable_path = './scrape.sh';

# construct a regex with alternatives for each element in an array
sub alternate_elems {
  my @array = @_;
  my $builder = "(";
  foreach (@array) {
    $builder .= $_ . "|";
  }
  # remove trailing extra |
  $builder = substr($builder, 0, length($builder) - 1);
  $builder .= ")";
  return qr/$builder/;
}

# extract database name from request
my $target_db = '';
my @matches = grep(/^db$/, $query->param);
my $num_matches = @matches;
if ($num_matches != 1) {
  die "Must specify database to scrape exactly once.";
} else {
  $target_db = $query->param('db');
}

# untiant database name
my @scrapers = &Local::Scrapers::scrapers();
my $expected_databases = &alternate_elems(@scrapers);
if ($target_db =~ $expected_databases) {
  $target_db = $1
} else {
  die "Scraper received unmatched data. Possible breakin attempt:$!";
}

# get parameter hash corresponding to target database
my %param_types = %{&Local::Scrapers::get_scraper_params($target_db)};

# construct parameter scrubber from expected parameter hash
my $expected_params = &alternate_elems(keys %param_types);

# untaint a field by testing if it was an expected scraper parameter.
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

# untaint a parameter by testing if it fits the expected format.
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

my @cmd = ($executable_path, $target_db);
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

# Make %ENV safer
$ENV{PATH} = "/bin:/usr/bin";
delete @ENV{qw(IFS CDPATH ENV BASH_ENV)};

print "Content-type: application/json\n\n";
system(@cmd);
