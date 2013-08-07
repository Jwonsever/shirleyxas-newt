#!/usr/bin/env python
#
# Simple proxy for the scraper at matzo.lbl.gov.
# Parameter scrubbing will be done there.
# TODO: look into what could go wrong by not scrubbing parameters here.

print "Content-Type: text/plain"
print ""

import cgi
import cgitb
import urllib
import urllib2

cgitb.enable()

form = cgi.FieldStorage()

# host of scraper
host = 'http://matzo.lbl.gov/'
# path to host's scraper
scraper = 'scrape/scrape.cgi'

# construct parameter dict
params = {}
for param in form.keys():
    params[param] = form[param].value

# make GET request
url_values = urllib.urlencode(params)
url = host + scraper + '?' + url_values
response = urllib2.urlopen(url)
print response.read()
