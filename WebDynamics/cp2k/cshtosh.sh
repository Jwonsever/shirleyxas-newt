#!/bin/sed -f
# csh2sh
# Converts csh scripts into sh
# (Original author unknown)
1 s/^#!\/bin\/csh/#!\/bin\/sh/
s/\<setenv\> *\([^        ]*\)[   ]*/export \1=/g
s/\<set\> *\([^=  ]*\)[   ]*=[    ]*/\1=/g
s/\<set\> *\([^   ]*\)/\1=true/g
s/\<if\> *(\(.*\)) *then\>/if [\1]; then/g
s/\<if\> *(\(.*\))\>/[\1] \&\&/g
#s/\<if\> *\[ *\{ *\([^}]*\)\} *]/if \1/g
s/\<endif\>/fi/g
s/\<case\> *\([^ :]*\) *:/\1)/g
s/\<default\> *:/*)/g
s/\<switch\> *(\(.*\))/case \1 in/g
s/\<breaksw\>/;;/g
s/\<endsw\>/esac/g
s/\<source\>/./g
s/\<foreach\> *\([^(]*\)(\(.*\))\>/for \1 in \2; do/g
s/\<while\> *(\(.*\))\>/while [\1]; do/g
s/\<end\>/done/g
s/\\!/!/g
s/>\& *\([a-zA-Z_\$]*\)/> \1 2>\&1/g
s/|\&/2>\&1 |/g
s/$argv/$*/g
s/$#argv/$#/g
s/\([a-zA-Z]*\)=\$</read \1/g
s/echo=true/set -x/g
s/verbose=true/set -v/g
s/ *;/;/g 