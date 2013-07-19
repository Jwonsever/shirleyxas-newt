#!/bin/sh

#./scrape.cgi > stdout.log 2> stderr.log
./scrape.cgi | tee stdout.log
#cat stdout.log
