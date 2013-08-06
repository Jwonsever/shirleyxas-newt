# Perl module that holds:
#   - path to each known scraper.
#   - expected parameters for each scraper.
#   - acceptable form for each parameter.

package Local::Scrapers;

use strict;
use warnings;

our $VERSION = '1.00';
use base 'Exporter';
our @EXPORT_OK = qw(scrapers get_scraper_params);


# all known scrapers.
# if you make a new scraper, add its name here.
my @scrapers_list = qw(ICSD AMCSD);
sub scrapers {
  return @scrapers_list;
}


# resusable regex patterns.
# name segments indicate possible values.
# each one trims surrounding whitespace.
my %regexes = (
  'num' => qr/^\s*(\d*)\s*$/,
  'alpha_num' => qr/^\s*([a-zA-Z\d]*?)\s*$/,
  'alpha_space' => qr/^\s*([a-zA-Z\s]*?)\s*$/,
  'alpha_space_comma' => qr/^\s*([a-zA-Z\s,]*?)\s*$/,
  'alpha_num_space' => qr/^\s*([a-zA-Z\d\s]*?)\s*$/,
  'alpha_num_space_paren' => qr/^\s*([a-zA-Z\d\s\(\)]*?)\s*$/,
  'alpha_num_dot_paren_comma' => qr/^\s*([a-zA-Z\d\.\(\),]*?)\s*$/,
  'alpha_space_paren_dash' => qr/^\s*([a-zA-Z\s\(\)\-]*?)\s*$/,
  'alpha_num_space_squote_dquote' => qr/^\s*([a-zA-Z\d\s'"]*?)\s*$/,
  'alpha_space_paren_dash_comma' => qr/^\s*([a-zA-Z\s\(\)\-,]*?)\s*$/,
  'alpha_num_score_space_dash_slash_equal_paren' => qr/^\s*([\w\s\-\/=\(\)]*?)\s*$/
);

# possible parameters for ICSD scraper,
# and format for each.
my %icsd_params = (
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

# possible parameters for AMCSD scraper,
# and format for each.
my %amcsd_params = (
  'mineral' => $regexes{'alpha_space_paren_dash'},
  'author' => $regexes{'alpha_space_comma'},
  'chemistry' => $regexes{'alpha_space_paren_dash_comma'},
  'cellParam' => $regexes{'alpha_num_score_space_dash_slash_equal_paren'},
  'diffraction' => $regexes{'alpha_num_dot_paren_comma'},
  'general' => $regexes{'alpha_num_space_squote_dquote'}
);

# all known databases mapped to their parameter-and-format hash.
# (rather, a reference to the hash.)
my %db_params = (
  'ICSD' => \%icsd_params,
  'AMCSD' => \%amcsd_params
);

# get parameter-and-format hash for given database name
sub get_scraper_params {
  my $target_db = $_[0];
  return $db_params{$target_db};
}

# end of module
1;
