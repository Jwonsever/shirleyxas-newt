#!/bin/sh
#
# Simple wrapper for the python web crawler.
# See the python file for usage.

# load needed modules
module load qt python/2.7.3

# run scraper
exec python2.7 crawl/icsdCrawler.py "$@"
