#!/bin/bash
#
# Loads modules required to run python scraper

module load qt python/2.7.3 2>&1
# somehow, during easy_install, the libraries didn't get moved. Fix:
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/common/usg/python/2.7.3/lib/python2.7/site-packages/PySide-1.2.0-py2.7.egg/PySide/
