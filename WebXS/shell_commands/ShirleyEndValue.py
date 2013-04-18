#/usr/bin/env python
import sys
import os
import fileinput

fileinput.close()

path = sys.argv[1]
nscfiles = [];
spectfiles = [];

os.chdir(path + "/XAS/")
for r,d,f in os.walk("."):
    for files in f:
        if files.endswith("nscf.out"):
             nscfiles.append(os.path.join(r,files))
        if files.endswith("xas.5.xas"):
             spectfiles.append(os.path.join(r,files))

minEv = 10000;
minDelta = 10000;
linebefore = ""
line2before = ""

#find base shift
for f in nscfiles:
    for line in fileinput.input(f):
        if "the Fermi energy" in line:
            value = line2before.split(" ")[-1]
            minEv = min(minEv,float(value))
        line2before = linebefore
        linebefore = line

#add delta shift
for f in spectfiles:
    for line in fileinput.input(f):
        if "delta shift" in line:
            value = line.split(" ")[-2]
            minDelta = min(minDelta,float(value))

print minEv + minDelta
