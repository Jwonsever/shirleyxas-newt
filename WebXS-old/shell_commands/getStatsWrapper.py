#!/usr/bin/env python
import sys
import os
import getStats

def main():
    if (len(sys.argv) < 2):
        print 'bad arguments'
        exit(0)

    direct = sys.argv[1]
    os.chdir(direct)
    fullPath = os.getcwd();
    
    header = "JobName:" + "Hours: " +  "Processors: " +  "Procs per Pool, Shirley Diag: " + "Number of atoms: "
    header += "Number of Electrons: " + "Number of Concurent Jobs (Usually equal to number of excited atoms): "
    header += "N-Band FAC: " + "FFT Grid: " + "G-Vectors (FFT): " + "Smooth Grid: "+ "G-Vectors (Smooth): "
    header += "Cell Volume: " + "Kohn-Sham Wavefunctions: " + "Maximum per-process Memory: " + "Disk usage: "
    header += "Crash?: " + "Snapshots: ";

    print header

    for dir in os.listdir("."):
        path = fullPath + "/" + dir
        try:
            output = getStats.main(path, dir, getStats.NO_PRINT)
            print output
        except:
            print >> sys.stderr, "Failed for this job: " + path

if __name__ == '__main__':
    main()
