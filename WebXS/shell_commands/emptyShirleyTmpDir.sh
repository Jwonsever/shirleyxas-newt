#!/bin/bash

cd /global/project/projectdirs/mftheory/www/james-xs/Shirley-data/tmp
find . -atime +30 -exec rm {} \;
