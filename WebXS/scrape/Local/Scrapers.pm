# Perl module that holds:
#   - path to each known scraper.
#   - expected parameters for each scraper.
#   - acceptable form for each parameter.

use strict;
use warnings;

our $VERSION = '1.00';
use base 'Exporter';
our @EXPORT = qw(get_scraper_path get_scraper_params);


sub get_scraper_path {
  my $target_db = $_[0];
  return $scraper_paths{$target_db};
}

my %scraper_paths = (
  'icsd' => './scrape_icsd.sh'
);


sub get_scraper_params {
  my $target_db = $_[0];
  return $db_params{$target_db};
}

my %db_params = (
  'icsd' => \%icsd_params
}

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

# each one trims whitespace
my %regexes = (
  'num' => qr/^\s*(\d*)\s*$/,
  'alpha_num' => qr/^\s*([a-zA-Z\d]*?)\s*$/,
  'alpha_space' => qr/^\s*([a-zA-Z\s]*?)\s*$/,
  'alpha_num_space' => qr/^\s*([a-zA-Z\d\s]*?)\s*$/,
  'alpha_num_space_paren' => qr/^\s*([a-zA-Z\d\s\(\)]*?)\s*$/,
  'alpha_space_paren_dash' => qr/^\s*([a-zA-Z\s\(\)-]*?)\s*$/
);

# end of module
1;
