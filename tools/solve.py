#!/usr/bin/env python

from os import walk
from natsort import natsorted, ns
import multiprocessing
import subprocess
import sys

if(len(sys.argv)) < 2:
    print "Specify executable name!\n"
    sys.exit(0)

num_of_cores = multiprocessing.cpu_count()
archer_data = "archer_data"

executable = sys.argv[1]

f = []
for (dirpath, dirnames, filenames) in walk(archer_data):
    f.extend(filenames)
    break

files_per_core = int(len(f) / (num_of_cores - 1))
rest_file_per_core = int(len(f) - (files_per_core * (num_of_cores - 1)))

f = natsorted(f)

processes = []

print "ARCHER RACES:"
for c in range(0, num_of_cores):
    if(c != (num_of_cores - 1)):
        lb = c * (files_per_core)
        ub = lb + files_per_core
        # print c,":",lb,ub
        # print f[lb:ub]
        lst = f[lb:ub]
        lst.append(archer_data)
        lst.append(executable)
        processes.append(subprocess.Popen(["solve_milp.py"] + lst))
    else:
        lb = len(f) - rest_file_per_core
        ub = len(f)
        # print c,":",lb,ub
        # print f[lb:ub]
        lst = f[lb:ub]
        lst.append(archer_data)
        lst.append(executable)
        processes.append(subprocess.Popen(["solve_milp.py"] + lst))

exit_codes = [p.wait() for p in processes]
sys.exit(0)
# print exit_codes