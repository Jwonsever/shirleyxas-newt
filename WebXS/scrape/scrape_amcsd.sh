#!/bin/bash -l
#
# Simple wrapper for the python web crawler.
# See the python file for usage.

# load needed modules
. ./load_modules.sh

# run scraper
exec python2.7 crawl/amcsdCrawler.py "$@"
