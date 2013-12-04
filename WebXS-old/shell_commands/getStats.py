#!/usr/bin/env python
import sys
import os

STD_PRINT = 1
NO_PRINT = 0

def main(path, pathname, printops=STD_PRINT):

    import os
    import fileinput
    import re
    import math
    from operator import itemgetter

    os.chdir(path)

    

    if (pathname[-1] == "/"):
        pathname = pathname[0:-1]

    ref = 0
    refprocs = 0
    xas = 0
    xasprocs = 0
    anal = 0
    state = 0
    
    for line in open('ref.out').readlines():
        if (line.find("Used Resources") != -1):
            tmp = line.split(":")
            ref += int(tmp[-3][-2:]) * 3600 + int(tmp[-2]) * 60 + int(tmp[-1])
        if (line.find("total number of processors") != -1):
            tmp = line.split(" ")
            refprocs = int(tmp[-1])
    for line in open('xas.out').readlines():
        if (line.find("Used Resources") != -1):
            tmp = line.split(":")
            xas += int(tmp[-3][-2:]) * 3600 + int(tmp[-2]) * 60 + int(tmp[-1])
        if (line.find("total number of processors") != -1):
            tmp = line.split(" ")
            xasprocs = int(tmp[-1])
    for line in open('anal.out').readlines():
        if (line.find("Used Resources") != -1):
            tmp = line.split(":")
            anal += int(tmp[-3][-2:]) * 3600 + int(tmp[-2]) * 60 + int(tmp[-1])
                
    if ("state.out" in os.listdir(".")):
        for line in open('state.out').readlines():
            if (line.find("Used Resources") != -1):
                tmp = line.split(":")
                state += int(tmp[-3][-2:]) * 3600 + int(tmp[-2]) * 60 + int(tmp[-1])

    rawtime = ref * refprocs + xas * xasprocs + anal * 24 + state * 24
    rawhrs = float(rawtime)/3600
    


    ppp = 0
    nJob = 0
    nBnd = 0
    numAtoms = 0
    numElec = 0
    maxMem = 0
    FFTGrid = ""
    FFTGV = ""
    SmoothGrid = ""
    SmoothGV = ""
    vol = 0
    WaveFunctions = ""

    for dir in os.listdir("./XAS/"):
        if (dir.find(pathname) != -1):
            os.chdir("./XAS/" + dir)
            for dir in os.listdir("."):
                if (len(dir)< 6 and dir != "GS"):
                    os.chdir("./"+dir)
                    for dir in os.listdir("."):
                        if (dir.find(".scf.out") != -1):
                            for line in open(dir).readlines():
                                if (line.find("dynamical memory:") != -1):
                                    tmp = line.split(" ")
                                    if (float(tmp[-2]) > maxMem):
                                        maxMem = float(tmp[-2])
                                if (line.find("unit-cell volume") != -1):
                                    tmp = line.split(" ")
                                    vol = float(tmp[-2])
                                if (line.find("FFT grid:") != -1):
                                    tmp = line.split(":")
                                    FFTGrid = tmp[-1][1:-1]
                                    tmp =  line.replace("(", "( ")
                                    tmp = re.sub("\s+"," ",tmp).split("(")
                                    tmp = tmp[1].split(" ")
                                    FFTGV = tmp[1]
                                if (line.find("smooth grid:") != -1):
                                    tmp = line.split(":")
                                    SmoothGrid = tmp[-1][1:-1]
                                    tmp = line.replace("(", "( ")
                                    tmp = re.sub("\s+"," ",tmp).split("(")
                                    tmp = tmp[1].split(" ")
                                    SmoothGV = tmp[1]
                                if (line.find("Kohn-Sham Wavefunctions") != -1):
                                    line = re.sub(r'\s+', ' ',line)
                                    tmp = line.split(" ")
                                    WaveFunctions = tmp[-4] + tmp[-3] + tmp[-2] + tmp[-1]
                    os.chdir("..")
                elif (dir == "GS"):
                    os.chdir("./"+dir)
                    for line in open("./Input_Block.in").readlines():
                        if (line.find("NELEC=") != -1):
                            numElec = int(float(line.split("=")[-1][0:-1]))
                        if (line.find("NBND_FAC=") != -1):
                            nBnd = int(float(line.split("=")[-1][0:-1]))
                        if (line.find("PPP=") != -1):
                            ppp = int(float(line.split("=")[-1][0:-1]))
                        if (line.find("NAT=") != -1):
                            numAtoms = int(float(line.split("=")[-1][0:-1]))
                        if (line.find("NJOB=") != -1):
                            nJob = int(float(line.split("=")[-1][0:-1]))
                    os.chdir("..")


    os.chdir(path)
    


    def getFolderSize(folder):
        total_size = os.path.getsize(folder)
        for item in os.listdir(folder):
            itempath = os.path.join(folder, item)
            if os.path.isfile(itempath):
                total_size += os.path.getsize(itempath)
            elif os.path.isdir(itempath):
                total_size += getFolderSize(itempath)
        return total_size


    #Finds First Instance Of A File with name in directory
    def deepFind(directory, filename):
        if os.path.isdir(directory):
            os.chdir(directory)
            for dir in os.listdir("."):
                result = deepFind(dir, filename)
                if (result): 
                    return result
            os.chdir("..")
        else:
            if (directory == filename):
                return os.getcwd()
        return False


    os.chdir(path)
    snapshots = 0
    for dir in os.listdir("."):
        if (re.search(".*\.xyz$", dir)):
              snapshots += 1

    if (printops):
        print "Jobname: " + pathname
        print "Hours: " + str(rawhrs)
        print "Processors: " + str(xasprocs)
        print "Procs per Pool, Shirley Diag: " + str(ppp)
        print "Number of atoms: " + str(numAtoms)
        print "Number of Electrons: " + str(numElec) 
        print "Number of Concurent Jobs (Usually equal to number of excited atoms): " + str(nJob)
        print "N-Band FAC: " + str(nBnd)
        print "FFT Grid: " + FFTGrid
        print "G-Vectors (FFT): " + FFTGV
        print "Smooth Grid: " + SmoothGrid 
        print "G-Vectors (Smooth): " + SmoothGV
        print "Cell Volume: " + str(vol) + " (a.u.)^3"
        print "Kohn-Sham Wavefunctions: " + WaveFunctions
        print "Maximum per-process Memory: " + str(maxMem) + " Mb"
        print "Disk usage: " + str(getFolderSize(".")/1048576) + "Mb"
        print "Crash?: " + str(deepFind(".", "CRASH"))
        print "Snapshots: " + str(snapshots)
    else:
        output = pathname + "\t"
        output += str(rawhrs) + "\t"
        output += str(xasprocs) + "\t"
        output += str(ppp) + "\t"
        output += str(numAtoms) + "\t"
        output += str(numElec)  + "\t"
        output += str(nJob) + "\t"
        output += str(nBnd) + "\t"
        output += FFTGrid + "\t"
        output += FFTGV + "\t"
        output += SmoothGrid  + "\t"
        output += SmoothGV + "\t"
        output += str(vol) + " (a.u.)^3" + "\t"
        output += WaveFunctions + "\t"
        output += str(maxMem) + " Mb" + "\t"
        output += str(getFolderSize(".")/1048576) + "Mb" + "\t"
        output += str(deepFind(".", "CRASH")) + "\t"
        output += str(snapshots) + "\t"
        return output

if __name__ == '__main__':
    if (len(sys.argv) < 2):
        print 'bad arguments'
        exit(0)

    pname = sys.argv[1]
    if (pname[-1] == "/"):
        pname = pname[0:-1]
    os.chdir(pname)
    bname = os.path.basename(pname)
    main(pname, bname)
