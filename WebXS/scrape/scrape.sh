#!/bin/sh
#
# Simple wrapper for the python web crawler.
# See the python file for usage.

# load needed modules
module load qt python/2.7.3
# somehow, during easy_install, the libraries didn't get moved. Fix:
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/common/usg/python/2.7.3/lib/python2.7/site-packages/PySide-1.2.0-py2.7.egg/PySide/

# run scraper
exec python2.7 crawl/icsdCrawler.py "$@"
