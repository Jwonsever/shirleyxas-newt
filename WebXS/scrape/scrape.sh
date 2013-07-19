#!/bin/sh
#
# Simple wrapper for the python web crawler.
# See the python file for usage.

echo "called"

# load needed modules
echo $PATH
module load qt python/2.7.3 2>&1
# somehow, during easy_install, the libraries didn't get moved. Fix:
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/common/usg/python/2.7.3/lib/python2.7/site-packages/PySide-1.2.0-py2.7.egg/PySide/

echo "modules loaded"
echo $PATH
#echo `which python2.7 2>> stderr.log`

# run scraper
echo `python2.7 crawl/icsdCrawler.py "$@" 2>> stderr.log` 2>> stderr.log
