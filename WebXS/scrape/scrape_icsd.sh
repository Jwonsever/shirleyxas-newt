#!/bin/bash -l
#
# Simple wrapper for the python web crawler.
# See the python file for usage.

echo "called"

# load needed modules
echo $PATH
. ./load_modules.sh

echo "modules loaded"
echo $PATH
echo `which python2.7 2>&1`

# run scraper
echo `python2.7 crawl/icsdCrawler.py "$@" 2>&1`
