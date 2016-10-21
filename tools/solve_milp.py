#!/usr/bin/env python

from collections import defaultdict
from enum import Enum
from itertools import combinations
from operator import itemgetter
from optlang import Model, Variable, Constraint, Objective
from os.path import expanduser
from psutil import virtual_memory
from swiglpk import *
import errno
import math
import os
import random
import re
import shutil
import subprocess
import sys

# LIMIT=0

symbolizer = 'which llvm-symbolizer'
process = subprocess.Popen(symbolizer.split(), stdout=subprocess.PIPE)
path = process.communicate()

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as e:
        if e.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise

def create_milp(problem_name, info, filename):
    ia = intArray(1+1000)
    ja = intArray(1+1000)
    ar = doubleArray(1+1000)
    lp = glp_create_prob()
    glp_set_prob_name(lp, problem_name)
    glp_set_obj_dir(lp, GLP_MIN)
    row = 1
    col = 1
    min = 9999999999999999
    max = 0
    i = 1

    # tid, address, count, size, type, pc, barrier
    for item in info:
      for v in item:
        # min and max bounds
        if(v[1] < min):
            min = v[1]
        if((v[1] + (v[2] * (1 << v[3]))) > max):
            max = v[1] + (v[2] * (1 << v[3]))
        size = v[3]
    offset = min
    min = 0
    LIMIT = ((max - offset) / (2 **size)) + 1
    max = max - offset

    glp_add_cols(lp, 1)
    glp_set_col_name(lp, col, "i")
    glp_set_obj_coef(lp, col, 0)
    glp_set_col_kind(lp, col, GLP_IV)
    glp_set_col_bnds(lp, col, GLP_DB, int(min), int(max) / (2 ** size))
    col += 1

    # tid, address, count, size, type, pc, barrier
    idx = 0
    t_col = []
    for item in info:
        t_col.append([])
        for v in item:
            # columns (variables)
            glp_add_cols(lp, 1)
            glp_set_col_name(lp, col, "i" + str(v[0]) + AccessTypeVar[v[4]] + "_" + str(idx))
            glp_set_col_bnds(lp, col, GLP_DB, (v[1] - offset) / (1 << v[3]), ((v[1] - offset) + v[2] * (1 << v[3])) / (1 << v[3]))
            glp_set_col_kind(lp, col, GLP_IV)
            col += 1
            glp_add_cols(lp, 1)
            glp_set_col_name(lp, col, "T" + str(v[0]) + AccessTypeVar[v[4]] + "_" + str(idx))
            glp_set_col_bnds(lp, col, GLP_DB, 0, 1)
            glp_set_obj_coef(lp, col, 1)
            glp_set_col_kind(lp, col, GLP_BV)
            t_col[idx].append(col)
            col += 1
            # rows (constraints)
            glp_add_rows(lp, 1)
            glp_set_row_name(lp, row, "T" + str(v[0]) + AccessTypeVar[v[4]] + "_" + str(idx) + "_1")
            glp_set_row_bnds(lp, row, GLP_UP, 0, LIMIT)
            ia[i] = row; ja[i] = 1; ar[i] = -1;
            i += 1
            ia[i] = row; ja[i] = col - 2; ar[i] = 1;
            i += 1
            ia[i] = row; ja[i] = col - 1; ar[i] = LIMIT;
            i += 1
            row += 1
            glp_add_rows(lp, 1)
            glp_set_row_name(lp, row, "T" + str(v[0]) + AccessTypeVar[v[4]] + "_" + str(idx) + "_2")
            glp_set_row_bnds(lp, row, GLP_UP, 0.0, LIMIT)
            ia[i] = row; ja[i] = 1; ar[i] = 1;
            i += 1
            ia[i] = row; ja[i] = col - 2; ar[i] = -1;
            i += 1
            ia[i] = row; ja[i] = col - 1; ar[i] = LIMIT;
            i += 1
            row += 1
        glp_add_rows(lp, 1)
        glp_set_row_name(lp, row, "exactly" + str(idx) + "_1")
        glp_set_row_bnds(lp, row, GLP_UP, 0, 1)
        for t in t_col[idx]:
            ia[i] = row; ja[i] = t; ar[i] = 1;
            i += 1
        row += 1
        glp_add_rows(lp, 1)
        glp_set_row_name(lp, row, "exactly" + str(idx) + "_1")
        glp_set_row_bnds(lp, row, GLP_LO, 1, 0)
        for t in t_col[idx]:
            ia[i] = row; ja[i] = t; ar[i] = 1;
            i += 1
        row += 1
        idx += 1

    j = 0
    for t in range(0, len(t_col[0])):
        glp_add_rows(lp, 1)
        glp_set_row_name(lp, row, "atmost" + str(j))
        glp_set_row_bnds(lp, row, GLP_UP, 0, 1)
        ia[i] = row; ja[i] = t_col[0][t]; ar[i] = 1;
        i += 1
        ia[i] = row; ja[i] = t_col[1][t]; ar[i] = 1;
        i += 1
        row += 1
        j += 1
    row += 1
    glp_load_matrix(lp, i - 1, ia, ja, ar)
    parm = glp_iocp()
    parm.presolve = GLP_ON
    parm.msg_lev = GLP_MSG_OFF
    parm.br_tech = GLP_BR_DTH
    parm.bt_tech = GLP_BT_BLB
    parm.pp_tech = GLP_PP_ALL
    parm.fp_heur = GLP_OFF
    parm.gmi_cuts = GLP_OFF
    parm.mir_cuts = GLP_OFF
    parm.cov_cuts = GLP_OFF
    parm.clq_cuts = GLP_OFF
    parm.tol_parm = 1e-5
    parm.tol_obj = 1e-7
    parm.mip_gap = 0.0
    parm.tol_int = 1e-5
    parm.tm_lim = 65000
    parm.out_frq = 5000
    parm.out_dly = 10000
    parm.cb_size = 0
    parm.binarize = GLP_OFF
    # res = glp_simplex(lp, None)
    res = glp_intopt(lp, parm)
    print "Result:",res,"[",0,GLP_EBOUND,GLP_EROOT,GLP_ENOPFS,GLP_ENODFS,GLP_EFAIL,GLP_EMIPGAP,GLP_ETMLIM,GLP_ESTOP,"]"
    glp_write_lp(lp, None, filename + "_" + problem_name + ".lp")
    glp_print_mip(lp, filename + "_" + problem_name + ".sol")
    if(res == 0):
        # Print Solution
        # Z = glp_mip_obj_val(lp)
        # i = (int(glp_mip_col_val(lp, 1)) * (1 << size)) + offset
        # print "Z =", str(int(Z))
        # print "i =", str(hex(int(i)))
        # for c in range(2,col):
        #     name = glp_get_col_name(lp, c)
        #     if("i" not in name):
        #         print name + "=" + str(int(glp_mip_col_val(lp, c)))
        #     else:
        #         print name + "=" + hex((int(glp_mip_col_val(lp, c)) * (1 << size)) + offset)
        # Print Solution

        # Print Race
        i = (int(glp_mip_col_val(lp, 1)) * (1 << size)) + offset
        racing_threads = list()
        count = 0
        for c in range(2,col):
          name = glp_get_col_name(lp, c)
          if("T" in name):
            if(int(glp_mip_col_val(lp, c)) == 1):
                thread = name.replace("T", "").split("_")
                racing_threads.append((int(thread[0]), AccessTypeVar.index("_" + thread[1]), int(thread[2])))
                count += 1
                if(count == 2):
                    break
        # tid, address, count, size, type1, pc, barrier
        array_race_list = []
        accesses = []
        for t in racing_threads:
            val = info[t[2]]
            for v in val:
                if((t[0] == v[0]) and (t[1] == v[4])):
                   accesses.append(v)
                   break
        list1 = [accesses[0][0], i]
        list1.extend(accesses[0][3:])
        array_race_list.append(tuple(list1))
        list2 = [accesses[1][0], i]
        list2.extend(accesses[1][3:])
        array_race_list.append(tuple(list2))

        printarrayraces(array_race_list)
        # Print Race
        glp_delete_prob(lp)

def create_milp2(problem_name, info):
    constraints = []
    bin_variables = []
    min = LIMIT
    max = 0
    for v in info:
        # min and max bounds
        if(v[1] < min):
            min = v[1]
        if((v[1] + (v[2] * (1 << v[3]))) > max):
            max = v[1] + (v[2] * (1 << v[3]))

    i = Variable('i', lb=min, up=max, type='integer')
    # tid, address, count, size, type1, type2, pc1, pc2
    for v in info:
        i_T = Variable('i' + str(v[0]), lb=v[1], ub=v[1] + (v[2] * (1 << v[3])), type='integer')
        T = Variable('T' + str(v[0]), lb=0, ub=1, type='binary')
        bin_variables.append(T)
        constraints.append(Constraint(i_T - i + LIMIT * T, lb = LIMIT))
        constraints.append(Constraint(i - i_T + LIMIT * T, lb = LIMIT))

    constraints.append(Constraint(sum(v for v in bin_variables), ub = 2))
    obj = Objective(sum(v for v in bin_variables), direction='min')

    model = Model(name=problem_name)
    model.objective = obj
    model.configuration.presolve = True
    model.add(constraints)

    status = model.optimize()

    print "status:", model.status
    print "objective value:", model.objective.value
    for var_name, var in model.variables.iteritems():
        if("i" in var_name):
            print var_name, "=", hex(int(var.primal))
        else:
            print var_name, "=", var.primal

def printraces(race):
    command = path[0].rstrip() + ' -pretty-print' + ' < <(echo "' + executable + ' ' + hex(race[0][4]) + '")'
    proc = subprocess.Popen(command, shell=True, executable='/bin/bash',
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT)
    race0 = proc.stdout.readline()
    command = path[0].rstrip() + ' -pretty-print' + ' < <(echo "' + executable + ' ' + hex(race[1][4]) + '")'
    proc = subprocess.Popen(command, shell=True, executable='/bin/bash',
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT)
    race1 = proc.stdout.readline()
    print "--------------------------------------------------"
    print "WARNING: Archer: data race (program=" + executable + ")"
    print AccessTypeName[race[0][3]] + " of size " + str(1 << race[0][2]) + " at " + hex(race[0][1]) + " by thread T" + str(race[0][0]) + " in " + race0.rstrip()
    print AccessTypeName[race[1][3]] + " of size " + str(1 << race[1][2]) + " at " + hex(race[1][1]) + " by thread T" + str(race[1][0]) + " in " + race1.rstrip()
    print "--------------------------------------------------"
    print

def printarrayraces(race):
    # ((tid1, address1, size1, type1, pc1, barrier1), (tid2, address2, size2, type2, pc2, barrier))
    command = path[0].rstrip() + ' -pretty-print' + ' < <(echo "' + executable + ' ' + hex(race[0][4]) + '")'
    proc = subprocess.Popen(command, shell=True, executable='/bin/bash',
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT)
    race0 = proc.stdout.readline()
    command = path[0].rstrip() + ' -pretty-print' + ' < <(echo "' + executable + ' ' + hex(race[1][4]) + '")'
    proc = subprocess.Popen(command, shell=True, executable='/bin/bash',
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT)
    race1 = proc.stdout.readline()
    print "--------------------------------------------------"
    print "WARNING: Archer: data race (program=" + executable + ")"
    print AccessTypeName[race[0][3]] + " of size " + str(1 << race[0][2]) + " at " + hex(race[0][1]) + " by thread T" + str(race[0][0]) + " in " + race0.rstrip()
    print AccessTypeName[race[1][3]] + " of size " + str(1 << race[1][2]) + " at " + hex(race[1][1]) + " by thread T" + str(race[1][0]) + " in " + race1.rstrip()
    print "--------------------------------------------------"
    print


def find_array_races(dct, filename):
    # Array Races
    array_access_list = []
    milp_set = []

    for k,v in dct.iteritems():
        lst = []
        for k1,v1 in v.iteritems():
            # lst.append((int(k, 16), int(k1), int(v1[0], 16), int(v1[1]), int(v1[2]), int(v1[3]), int(v1[4], 16), int(v1[5], 16)))
            lst.append((int(k1), int(v1[0], 16), int(v1[1]), int(v1[2]), int(v1[3]), int(v1[4], 16), int(v1[5], 16)))
        array_access_list.append(lst)

    # tid, address, count, size, type, pc, barrier
    # (1, 24385376, 49, 3, 2, 4197942, 0)
    combinat = combinations(array_access_list, r = 2)
    for key1, key2 in combinat:
        # Same barrier interval and same access size?
        if((key1[0][6] == key2[0][6]) and (key1[0][3] == key2[0][3])):
            if((key1[0][4] == AccessType.unsafe_write) or
               (key2[0][4] == AccessType.unsafe_write) or
               ((key1[0][4] == AccessType.unsafe_read) and
                (key2[0][4] not in set([AccessType.unsafe_read,
                                        AccessType.atomic_read,
                                        AccessType.mutex_read]))) or
               ((key2[0][4] == AccessType.unsafe_read) and
                (key1[0][4] not in set([AccessType.unsafe_read,
                                        AccessType.atomic_read,
                                        AccessType.mutex_read]))) or
               ((key1[0][4] == AccessType.atomic_read) and
                (key2[0][4] not in set([AccessType.unsafe_read,
                                        AccessType.atomic_read,
                                        AccessType.atomic_write,
                                        AccessType.mutex_read]))) or
               ((key2[0][4] == AccessType.atomic_read) and
                (key1[0][4] not in set([AccessType.unsafe_read,
                                        AccessType.atomic_read,
                                        AccessType.atomic_write,
                                        AccessType.mutex_read]))) or
               ((key1[0][4] == AccessType.atomic_write) and
                (key2[0][4] not in set([AccessType.atomic_read,
                                        AccessType.atomic_write]))) or
               ((key2[0][4] == AccessType.atomic_write) and
                (key1[0][4] not in set([AccessType.atomic_read,
                                        AccessType.atomic_write]))) or
               ((key1[0][4] == AccessType.mutex_read) and
                (key2[0][4] not in set([AccessType.unsafe_read,
                                        AccessType.mutex_read,
                                        AccessType.mutex_write,
                                        AccessType.atomic_read]))) or
               ((key2[0][4] == AccessType.mutex_read) and
                (key1[0][4] not in set([AccessType.unsafe_read,
                                        AccessType.mutex_read,
                                        AccessType.mutex_write,
                                        AccessType.atomic_read]))) or
               ((key1[0][4] == AccessType.mutex_write) and
                (key2[0][4] not in set([AccessType.mutex_read,
                                        AccessType.mutex_write]))) or
               ((key2[0][4] == AccessType.mutex_write) and
                (key1[0][4] not in set([AccessType.mutex_read,
                                        AccessType.mutex_write])))):
                milp_set.append((tuple(key1), tuple(key2)))

    count = 0
    for v in milp_set:
        create_milp("milp_" + str(count), v, filename)
    # Array Races

def find_scalar_races(dict, filename):
    # Scalar Races
    scalar_access_set = set()

    for k,v in dict.iteritems():
        for k1,v1 in v.iteritems():
            scalar_access_set.add((int(k1), int(v1[0], 16), int(v1[1]), int(v1[2]) , int(v1[3], 16)))

    scalar_race_set = set()
    # (tid, address, size, type, pc)
    for key1, key2 in combinations(scalar_access_set, r = 2):
        if(key1[1] == key2[1]):
            if((key1[3] == key2[3] == AccessType.unsafe_read) or
               (key1[3] == key2[3] == AccessType.atomic_read) or
               (key1[3] == key2[3] == AccessType.mutex_read)):
                # print (key1, key2)
                # scalar_race_set.add((key1, key2))
                # print scalar_race_set
                pass
            # (tid1, address1, size1, type1, pc1, tid2, address2,  size2, type2, pc2)
            # address are the same, just for easy insertion on the set we repeat it
            elif((key1[0] != key1[2]) and
                 ((key1[3] == AccessType.unsafe_read) and
                  ((key2[3] == AccessType.unsafe_write) or
                   (key2[3] == AccessType.atomic_write) or
                   (key2[3] == AccessType.mutex_write))) or
                 ((key2[3] == AccessType.unsafe_read) and
                  ((key1[3] == AccessType.unsafe_write) or
                   (key1[3] == AccessType.atomic_write) or
                   (key1[3] == AccessType.mutex_write)))):
                scalar_race_set.add((key1, key2))
            elif((key1[0] != key1[2]) and
                 ((key1[3] == AccessType.unsafe_write) and
                  ((key2[3] == AccessType.unsafe_read) or
                   (key2[3] == AccessType.atomic_read) or
                   (key2[3] == AccessType.atomic_write) or
                   (key2[3] == AccessType.mutex_read) or
                   (key2[3] == AccessType.mutex_write))) or
                 ((key2[3] == AccessType.unsafe_write) and
                  ((key1[3] == AccessType.unsafe_read) or
                   (key1[3] == AccessType.atomic_read) or
                   (key1[3] == AccessType.atomic_write) or
                   (key1[3] == AccessType.mutex_read) or
                   (key1[3] == AccessType.mutex_write)))):
                scalar_race_set.add((key1, key2))
            elif((key1[0] != key1[2]) and
                 ((key1[3] == AccessType.atomic_read) and
                  ((key2[3] == AccessType.unsafe_write) or
                   (key2[3] == AccessType.mutex_write))) or
                 ((key2[3] == AccessType.atomic_read) and
                  ((key1[3] == AccessType.unsafe_write) or
                   (key1[3] == AccessType.mutex_write)))):
                scalar_race_set.add((key1, key2))
            elif((key1[0] != key1[2]) and
                 ((key1[3] == AccessType.atomic_write) and
                  ((key2[3] == AccessType.unsafe_read) or
                   (key2[3] == AccessType.unsafe_write) or
                   (key2[3] == AccessType.mutex_read) or
                   (key2[3] == AccessType.mutex_write))) or
                 ((key2[3] == AccessType.atomic_write) and
                  ((key1[3] == AccessType.unsafe_read) or
                   (key1[3] == AccessType.unsafe_write) or
                   (key1[3] == AccessType.mutex_read) or
                   (key1[3] == AccessType.mutex_write)))):
                scalar_race_set.add((key1, key2))
            elif((key1[0] != key1[2]) and
                 ((key1[3] == AccessType.mutex_read) and
                  ((key2[3] == AccessType.unsafe_write) or
                   (key2[3] == AccessType.atomic_write))) or
                 ((key2[3] == AccessType.mutex_read) and
                  ((key1[3] == AccessType.unsafe_write) or
                   (key1[3] == AccessType.atomic_write)))):
                scalar_race_set.add((key1, key2))
            elif((key1[0] != key1[2]) and
                 ((key1[3] == AccessType.mutex_write) and
                  ((key2[3] == AccessType.unsafe_read) or
                   (key2[3] == AccessType.unsafe_write) or
                   (key2[3] == AccessType.atomic_read) or
                   (key2[3] == AccessType.atomic_write))) or
                 ((key2[3] == AccessType.mutex_write) and
                  ((key1[3] == AccessType.unsafe_read) or
                   (key1[3] == AccessType.unsafe_write) or
                   (key1[3] == AccessType.atomic_read) or
                   (key1[3] == AccessType.atomic_write)))):
                scalar_race_set.add((key1, key2))

    if(len(scalar_race_set) > 0 or len(scalar_race_set) > 0):
        print "ARCHER RACES:"

    # llvm-symbolizer < <(echo "reduction 0x400fd7")
    # ((tid1, address1, size1, type1, pc1), (tid2, address2,  size2, type2, pc2))
    # WARNING: Archer: data race (program=executable_name)
    # Write of size 4 at 0x7ffdfdd803bc by thread T7 at line ...
    # Write of size 4 at 0x7ffdfdd803bc by thread T6 at line ...

    for race in scalar_race_set:
        printraces(race)
    # Scalar Races

class AccessType(Enum):
    none = 0
    unsafe_read = 1
    unsafe_write = 2
    atomic_read = 3
    atomic_write = 4
    mutex_read = 5
    mutex_write = 6
    nutex_read = 7
    nutex_write = 8

class AccessSize(Enum):
    size1 = 0
    size2 = 1
    size4 = 2
    size8 = 3
    size16 = 4

AccessTypeName = ['None', 'Read', 'Write', 'Atomic Read', 'Atomic Write', 'Critical Read', 'Critical Write']
AccessTypeVar = ['_n', '_r', '_w', '_ar', '_aw', '_cr', '_cw']

PARALLEL_BREAK = "PARALLEL_BREAK"
PARALLEL_START = "PARALLEL_START"
PARALLEL_END = "PARALLEL_END"
DATA_BEGIN = "DATA_BEGIN"
DATA_END = "DATA_END"
DATA = "DATA"

if(len(sys.argv)) < 3:
    print "Specify input file and executable!\n"
    sys.exit(0)

filename = sys.argv[1]
executable = sys.argv[2]

dir_log = "./archer_races"
shutil.rmtree(dir_log, ignore_errors=True)
mkdir_p(dir_log)
file_list = []
nf = None
newfile = ""
with open(filename, "r") as f:
    for line in f:
        if(line.startswith(PARALLEL_START)):
            string = re.search(r"\[([A-Za-z0-9,]+)\]", line)
            parallel_id = int(string.group(1))
            newfile = dir_log + "/" + "parallelregion_" + str(parallel_id)
            nf = open(newfile, "w")
            nf.write(line)
        elif(line.startswith(PARALLEL_BREAK)):
            file_list.append(newfile)
            nf.close()
        else:
            nf.write(line)
f.close()

for filename in file_list:
    print "Reading file " + filename + "..."
    parallel_id = 0
    tid = 0;
    parallel_level = 0
    array_dict = defaultdict(lambda : defaultdict(list))
    scalar_dict = defaultdict(lambda : defaultdict(list))
    with open(filename, "r") as f:
        for line in f:
            if(line.startswith(PARALLEL_BREAK)):
                break
            elif(line.startswith(PARALLEL_START)):
                string = re.search(r"\[([A-Za-z0-9,]+)\]", line)
                parallel_id = int(string.group(1))
            elif(line.startswith(PARALLEL_END)):
                parallel_id = 0
            elif(line.startswith(DATA_BEGIN)):
                string = re.search(r"\[([A-Za-z0-9,]+)\]", line)
                info = string.group(1).split(',')
                tid = info[2]
                parallel_level = info[3]
            elif(line.startswith(DATA_END)):
                tid = 0;
            elif(line.startswith(DATA)):
                string = re.search(r"\[([A-Za-z0-9,]+)\]", line)
                info = string.group(1).split(',')
                if(int(info[2]) > 1):
                    array_dict[info[0]][tid].extend(info[1:])
                    array_dict[info[0]][tid].extend(parallel_level)
                else:
                    scalar_dict[info[0]][tid].extend(info[1:2] + info[3:])
                    scalar_dict[info[0]][tid].extend(parallel_level)
    print "Checking for races..."
    # print array_dict
    find_array_races(array_dict, filename)
    # find_scalar_races(scalar_dict, filename)
