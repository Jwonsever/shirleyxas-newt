#/usr/bin/env python
import sys
import os
import fileinput
from re import match
from math import fabs
from operator import itemgetter

import topEvContributors
import ShirleyEndValue

#Quick Description.
#This file grabs all "relevant states" from a directory based on the spectrum

#Begins in *./XAS/Spectrum-'xx'

def main(argv = None):
    if argv is None:
        argv = sys.argv

    fileinput.close()

    if (len(argv) < 3):
        print 'bad arguments to FindNotableStates.py'
        exit(0)

    path = argv[1]
    molName = argv[2]

    #Rest of the args not currently used...
    #flags = argv[3]

    startEv=0
    endEv=0

    if (len(argv) >= 5):
        startEv = argv[4]
    else:
        startEv = -5

    if (len(argv) >= 6):
        endEv = argv[5]
    else:
        endEv = ShirleyEndValue.main(path) #reads from redline script

#Begin by moving in *./XAS/Spectrum-'xx'
    os.chdir(path + "/XAS/")
    dirnames = []
    for dirname in os.listdir('.'):
        if match("^Spectrum-.*$", dirname):
            dirnames.append(dirname);
        
    importantEvs = []
    
    for dir in dirnames:
        os.chdir(path+"/XAS/"+dir)
        #Run Avg File
        avgFilename = dir.replace("Spectrum", "Spectrum-Ave")
        avgFile = readFile(avgFilename)
        importantEvs.extend(localMaximums(avgFile, startEv, endEv).keys());
        
        #run Other files against avg file
        for fname in os.listdir('.'):
            if match("^.*.xas.5.xas$", fname):    
                importantEvs.extend(compareToAverage(fname, avgFile, startEv, endEv).keys());
                importantEvs.extend(localMaximums(readFile(fname), startEv, endEv).keys());
        
        os.chdir(path)
        
    fileinput.close()

    #remove duplicates
    importantEvs = list(set(importantEvs))
    goodStates = []
    for ev in importantEvs:
        goodStates.extend(topEvContributors.main([argv[0], path, molName, ev, .25, 5]))

    #remove duplicates
    goodStates = list(set(goodStates))
    
    return goodStates


def readFile(filename):
    filData = {};
    for line in fileinput.input([filename]):
        if fileinput.isfirstline():
            continue
        l = line.split()
        filData[l[0]] = l[1]

    fileinput.close()
    return filData

def localMaximums(values, startEv, endEv):
    lastEv = 0
    lastVal = 0
    maximums = {}

    for ev in values:
        if (float(ev) > endEv):
            continue

        fileVal = float(values[ev])
        if (fileVal > lastVal):
            lastVal = fileVal
            lastEv = ev
        else:
            maximums[lastEv] = lastVal
    
    return maximums

def compareToAverage(filename, averageFile, startEv, endEv):
    fileValues = readFile(filename)
    majorDifs = {}
    lastEv = 0
    lastDiff = 0

    for ev in fileValues:
        if (float(ev) > endEv):
            continue

        fileVal = float(fileValues[ev])
        avgVal = float(averageFile[ev])
        diff = fileVal - avgVal

        proportionalDiff = 0
        if (avgVal < 1.0e-20):
            continue #obviously no states here
        else:
            proportionalDiff = diff / avgVal
        
        if (proportionalDiff > 0.20):
            #Greater then 20% difference
            if (proportionalDiff > lastDiff):
                lastDiff = proportionalDiff
                lastEv = ev
            else:
                majorDifs[lastEv] = lastDiff

    return majorDifs

if __name__ == '__main__':
    out = main()
    for x in out:
        print x
