#!/bin/bash

#move to dir to clean
cd $1

find . -name "*.save" -exec rm -rf {} \;
find . -name "*.hamprj" -exec rm -rf {} \;
find . -name "*.hamloc" -exec rm -rf {} \;
#Possibly remove this one
#find . -name "*.cube" -exec rm -rf {} \;

find . -name "*.dat" -exec rm -rf {} \;
find . -name "*.xmat" -exec rm -rf {} \;
find . -name "*.stick*" ! -name "* 0" -exec rm -rf {} \;
find . -name "*.wcf*" -exec rm -rf {} \;
find . -name "*fort*" -exec rm -rf {} \;
find . -name "*.sdb*" -exec rm -rf {} \;
find . -name "*wfc*" -exec rm -rf {} \;

#Possibly send to archive script from here?