#!/bin/bash

cd /global/project/projectdirs/als/www/jack-area/Shirley-data/tmp
find . -atime +30 -exec rm {} \;
