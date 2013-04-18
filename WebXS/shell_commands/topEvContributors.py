#/usr/bin/env python
import sys
import os
import fileinput
from re import match
from math import fabs
from operator import itemgetter

fileinput.close()

if (len(sys.argv) < 4):
    print 'bad arguments'
    exit(0)

path = sys.argv[1]
molName = sys.argv[2]
activeEv = float(sys.argv[3])
ebars = .5 #finds peaks within 1 ev

os.chdir(path)
ls = os.listdir('.')

webdata = fileinput.input('webdata.in')
webdata.next()
webdata.next()
xasAtoms = webdata.next().split()
webdata.next()
models = int(webdata.next())
totalAtoms = str(int(webdata.next()))
webdata.close()
topBands = []

def addBand(properties):
    topBands.append(properties)

for model in range(models):
    for atom in xasAtoms:

        if match('^[A-Za-z]{1,2}$', atom):
            continue

        #take care of those wierd 0's in the names
        longatom = atom

        if (match('^[A-Za-z]{1,2}\d{1}$', atom) and len(totalAtoms) > 2):
            longatom = longatom[0:-1] + "00" + longatom[-1]
        elif (match('^[A-Za-z]{1,2}\d{1}$', atom) and len(totalAtoms) > 1):
            longatom = longatom[0:-1] + "0" + longatom[-1]

        if (match('^[A-Za-z]{1,2}\d{2}$', atom) and len(totalAtoms) > 2):
            longatom = longatom[0:-2] + "0" + longatom[-2:]

        fileloc = 'XAS/' + molName + '_' + str(model) + '/' + atom + '/'
        filename = fileloc+molName+'.'+longatom+"-XCH.xas.5.stick.  0"
        for line in fileinput.input(filename):
            l = line.split()
            state = int(l[0])
            kpoint = int(l[1])
            if (kpoint != 1):
                break
            ev = float(l[2])
            ostrength = float(l[3])

            if (fabs(ev - activeEv) > ebars):
                continue
            
            l.insert(0, model)
            l.insert(0, atom)
            l.pop()
            l.pop()
            l.pop()
            addBand(l)
        fileinput.close()

topBands.sort(key = lambda row: -1 * float(row[5]))
for l in topBands:
    print str(l[0]) + ',' + str(l[1]+1) + ',' + str(l[2]) + ',' + str(round(float(l[4]), 2)) + ',' + str(round(float(l[5])*1000000, 2))
            
fileinput.close()
