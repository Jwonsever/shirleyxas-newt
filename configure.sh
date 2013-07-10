#!/bin/bash

cd `dirname $0`

WHERE_AM_I=`pwd`

WHERE_AM_I=${WHERE_AM_I//\//\\\/}
sedCmd='s/TMP_HOME/'${WHERE_AM_I}'/g'
sed -in $sedCmd GlobalValues.in
sed -in $sedCmd GlobalValues.js
sed -in $sedCmd WebXS/xas_input/Input_Block.in


WHERE_AM_I=`pwd`
WEB_ADDR="http://portal.nersc.gov/"
WEB_ADDR_PATH=`echo $WHERE_AM_I | cut -d/ --output-delimiter=" " -f 1- | awk '{print $1"\/"$3"\/"$5;}'`
WEB_ADDR=$WEB_ADDR$WEB_ADDR_PATH

WEB_ADDR=${WEB_ADDR//\//\\\/}
sedCmd='s/TMP_ADDR/'${WEB_ADDR}'/g'
sed -in $sedCmd GlobalValues.in
sed -in $sedCmd GlobalValues.js
sed -in $sedCmd WebXS/xas_input/Input_Block.in


Chmod -R 755 *
Chmod 777 Shirley-data/tmp 