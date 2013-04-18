#/usr/bin/env python
import sys
import os
import fileinput

fileinput.close()

if (len(sys.argv) < 2):
    print 'bad arguments'
    exit(0)
    
terms = sys.argv[1:]

os.chdir("/project/projectdirs/als/www/Shirley-data/spectraDB/")

results = []
output = ""

for line in fileinput.input("index.dat"):
    if (line[0] == "#"):
        continue
      
    for term in terms:
        if term.lower() in line.lower():
            l = line.split()
            for x in l:
               output += x + " "
            output = output[:-1] + "\n"
    
fileinput.close()

if len(output) > 1:
    print output
else:
    print "No match"

