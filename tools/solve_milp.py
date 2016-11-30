#!/usr/bin/env python

from collections import defaultdict
from enum import Enum
from gurobipy import *
from itertools import combinations
from operator import itemgetter
from os.path import expanduser
from psutil import virtual_memory
from swiglpk import *
import errno
import math
import multiprocessing
import os
import random
import re
import shutil
import subprocess
import sys
from distutils.log import info

symbolizer = 'which llvm-symbolizer'
process = subprocess.Popen(symbolizer.split(), stdout = subprocess.PIPE)
path = process.communicate()
races_lines_pairs = set()

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as e:
        if e.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise

def create_milp_gurobi(problem_name, info, filename):
    try:
        # Create a new model
        m = Model(filename + "_" + problem_name)

        constraints = []
        bin_variables1 = []
        bin_variables2 = []

        min = 9999999999999999
        max = 0
        size = 0
        # tid, address, count, size, type, pc, barrier
        for item in info:
            for v in item:
                # min and max bounds
                if(v[1] < min):
                    min = v[1]
                if((v[1] + (v[2] * (1 << v[3]))) > max):
                    max = v[1] + (v[2] * (1 << v[3]))
                if(size == 0):
                    size = 1 << v[3]

        offset = min
        max = max - offset

        i = m.addVar(name = 'i', vtype=GRB.INTEGER, lb = 0, ub = max)

        # tid, address, count, size, type, pc, diff, osl, barrier
        idx = 0
        t_col = []
        t_col_almost = defaultdict(list)
        pc = 0
        for item in info:
            t_col.append([])
            n = m.addVar(name = 'n_' + str(idx), vtype = GRB.INTEGER, lb = 0, ub = max)
            for v in item:
                size = 1 << v[3]
                lbv = v[1] - offset
                ubv = v[1] - offset + (v[2] * size)
                suffix = str(v[0]) + AccessTypeVar[v[4]] + "_" + str(idx)
                i_T = m.addVar(name = 'i' + suffix, lb = lbv, ub = ubv, vtype = GRB.INTEGER)
                T = m.addVar(name = 'T' + suffix, vtype = GRB.BINARY, obj = 1)
                m.update()
                t_col[idx].append(T)
                t_col_almost[v[0]].append(T)
                m.addGenConstrIndicator(T, True, i_T - i, GRB.EQUAL, 0)
                m.addGenConstrIndicator(T, True, i_T - v[6] * n, GRB.EQUAL, 0)
            m.addConstr(sum(t for t in t_col[idx]), GRB.EQUAL, 1, "exactly" + str(idx))
            idx += 1

        j = 0
        len0 = len(t_col[0])
        len1 = len(t_col[1])
        length = len0 if len0 < len1 else len1
        for k, v in t_col_almost.iteritems():
            if(len(v) == 1):
                m.addConstr(v[0], GRB.LESS_EQUAL, 1, "atmost" + str(j))
            else:
                m.addConstr(v[0] + v[1], GRB.LESS_EQUAL, 1, "atmost" + str(j))
            j += 1

        m.update()
        m.write(directory + "/" + filename + "_" + problem_name + ".lp")
        m.setParam('OutputFlag', False)
        m.optimize()

        if((m.status == GRB.Status.OPTIMAL) or (m.status == GRB.Status.SUBOPTIMAL)):
            m.write(directory + "/" + filename + "_" + problem_name + ".sol")
            print directory + "/" + filename + "_" + problem_name + ".sol"

            # Print Race
            racing_threads = list()
            count = 0
            for v in m.getVars():
                if(v.varName == "i"):
                    i = int(v.x) + offset
                else:
                    name = v.varName
                    if("T" in name):
                        if(v.x == 1):
                            thread = name.replace("T", "").split("_")
                            racing_threads.append((int(thread[0]), AccessTypeVar.index("_" + thread[1]), int(thread[2])))
                            count += 1
                            if(count == 2):
                                break
            # tid, address, count, size, type1, pc, barrier
            array_race_list = []
            accesses = []
            # SIMONE: check this, probably need to loop and find the right one, they are not in order
            for t in racing_threads:
                val = info[t[2]]
                for v in val:
                    if((t[0] == v[0]) and (t[1] == v[4])):
                       accesses.append(v)
                       break
            if(len(accesses) > 1):
                list1 = [accesses[0][0], i]
                list1.extend(accesses[0][3:])
                array_race_list.append(tuple(list1))
                list2 = [accesses[1][0], i]
                list2.extend(accesses[1][3:])
                array_race_list.append(tuple(list2))

                printarrayraces(array_race_list)
            # # Print Race

    except GurobiError as gurobi_err:
        print 'Encountered a Gurobi error', gurobi_err
    except AttributeError as attr_err:
        print 'Encountered an attribute error', attr_err

def concurrent_scalar(set1, set2):
    # (8, 140737116268256, 2, 2, 6916502, ((0, 1), (7, 24)), 1)
    osl1 = set1[6]
    osl2 = set2[6]
    concurr = True
    # case1
    if(len(osl1) < len(osl2)):
        length = len(osl1)
        for i in range(length):
            if(osl1[i] != osl2[i]):
                break
        else:
            concurr = False
    elif(len(osl1) > len(osl2)):
        length = len(osl2)
        for i in range(length):
            if(osl1[i] != osl2[i]):
                break
        else:
            concurr = False
    else:
        # case2
        length = len(osl1)
        for i in range(length):
            if(osl1[i] != osl2[i]):
                if(osl1[i][1] == osl2[i][1]):
                    s = osl1[i][1]
                    if((i + 1 == length) or ((osl1[i][0] % s) == (osl2[i][0] % s))):
                        concurr = False
                break
    return concurr

def concurrent_array(set1, set2):
    osl1 = osl2 = 0
    for s1 in set1:
        for s2 in set2:
            if((s1[0] != s2[0]) and (s2[7] != osl1)):
                osl1 = s1[7]
                osl2 = s2[7]
                break
    if((osl1 == 0) or (osl2 == 0)):
       return False
    concurr = True
    # case1
    if(len(osl1) < len(osl2)):
        length = len(osl1)
        for i in range(length):
            if(osl1[i] != osl2[i]):
                break
        else:
            concurr = False
    elif(len(osl1) > len(osl2)):
        length = len(osl2)
        for i in range(length):
            if(osl1[i] != osl2[i]):
                break
        else:
            concurr = False
    else:
        # case2
        length = len(osl1)
        for i in range(length):
            if(osl1[i] != osl2[i]):
                if(osl1[i][1] == osl2[i][1]):
                    s = osl1[i][1]
                    if((i + 1 == length) or ((osl1[i][0] % s) == (osl2[i][0] % s))):
                        concurr = False
                break
    return concurr

def printraces(race):
    command = path[0].rstrip() + ' -pretty-print' + ' < <(echo "' + executable + ' ' + hex(race[0][4]) + '")'
    proc = subprocess.Popen(command, shell = True, executable = '/bin/bash',
                            stdin = subprocess.PIPE,
                            stdout = subprocess.PIPE,
                            stderr = subprocess.STDOUT)
    race0 = proc.stdout.readline()
    command = path[0].rstrip() + ' -pretty-print' + ' < <(echo "' + executable + ' ' + hex(race[1][4]) + '")'
    proc = subprocess.Popen(command, shell = True, executable = '/bin/bash',
                            stdin = subprocess.PIPE,
                            stdout = subprocess.PIPE,
                            stderr = subprocess.STDOUT)
    race1 = proc.stdout.readline()
    print "--------------------------------------------------"
    print "WARNING: Archer: scalar data race (program=" + executable + ")"
    print AccessTypeName[race[0][3]] + " of size " + str(1 << race[0][2]) + " at " + hex(race[0][1]) + " by thread T" + str(race[0][0]) + " in " + race0.rstrip()
    print AccessTypeName[race[1][3]] + " of size " + str(1 << race[1][2]) + " at " + hex(race[1][1]) + " by thread T" + str(race[1][0]) + " in " + race1.rstrip()
    print "--------------------------------------------------"
    print

def printarrayraces(race):
    # ((tid1, address1, size1, type1, pc1, barrier1), (tid2, address2, size2, type2, pc2, barrier))
    command = path[0].rstrip() + ' -pretty-print' + ' < <(echo "' + executable + ' ' + hex(race[0][4]) + '")'
    proc = subprocess.Popen(command, shell = True, executable = '/bin/bash',
                            stdin = subprocess.PIPE,
                            stdout = subprocess.PIPE,
                            stderr = subprocess.STDOUT)
    race0 = proc.stdout.readline()
    command = path[0].rstrip() + ' -pretty-print' + ' < <(echo "' + executable + ' ' + hex(race[1][4]) + '")'
    proc = subprocess.Popen(command, shell = True, executable = '/bin/bash',
                            stdin = subprocess.PIPE,
                            stdout = subprocess.PIPE,
                            stderr = subprocess.STDOUT)
    race1 = proc.stdout.readline()
    print "--------------------------------------------------"
    print "WARNING: Archer: array data race (program=" + executable + ")"
    print AccessTypeName[race[0][3]] + " of size " + str(1 << race[0][2]) + " at " + hex(race[0][1]) + " by thread T" + str(race[0][0]) + " in " + race0.rstrip()
    print AccessTypeName[race[1][3]] + " of size " + str(1 << race[1][2]) + " at " + hex(race[1][1]) + " by thread T" + str(race[1][0]) + " in " + race1.rstrip()
    print "--------------------------------------------------"
    print


def find_array_races(dct, filename):
    # Array Races
    array_access_list = []
    milp_set = []

    for k, v in dct.iteritems():
        lst = []
        for k1, v1 in v.iteritems():
            lst.append((int(k1), int(v1[0], 16), int(v1[1]), int(v1[2]), int(v1[3]), int(v1[4], 16), int(v1[5]), tuple(v1[6]), int(v1[7])))
        array_access_list.append(lst)

    # tid, address, count, size, type, pc, diff, osl, barrier
    combinat = combinations(array_access_list, r = 2)
    for key1, key2 in combinat:
        # (Concurrent through osl or same barrier interval) and same access size
        # tid, access, count, size, type, pc, diff, osl, barrier
        if((key1[0][3] == key2[0][3]) and ((key1[0][8] == key2[0][8]) or
            ((key1[0][8] != key2[0][8]) and concurrent_array(key1, key2)))):
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
        create_milp_gurobi("milp_" + str(count), v, filename)
        count += 1
    # Array Races

def find_scalar_races(dict, filename):
    # Scalar Races
    scalar_access_set = set()

    for k, v in dict.iteritems():
        for k1, v1 in v.iteritems():
            scalar_access_set.add((int(k1), int(v1[0], 16), int(v1[1]), int(v1[2]), int(v1[3], 16), int(v1[4]), tuple(v1[5]), int(v1[6])))

    scalar_race_set = set()
    # (tid, address, size, type, pc)
    for key1, key2 in combinations(scalar_access_set, r = 2):
        # tid, access, size, type, pc, osl, barrier
        if(key1[0] != key2[0]):
            # (8, 140737116268256, 2, 2, 6916502, ((0, 1), (7, 24)), 1)
            # if((key1[1] == key2[1]) and (key1[2] == key2[2]) and (key1[6] == key2[6])):
            # same size memory access, same barrier or different barrier but concurrent
            if((key1[1] == key2[1]) and (key1[2] == key2[2]) and
                ((key1[7] == key2[7]) or ((key1[7] != key2[7]) and concurrent_scalar(key1, key2)))):
                if((key1[3] == AccessType.unsafe_write) or
               (key2[3] == AccessType.unsafe_write) or
               ((key1[3] == AccessType.unsafe_read) and
                (key2[3] not in set([AccessType.unsafe_read,
                                        AccessType.atomic_read,
                                        AccessType.mutex_read]))) or
               ((key2[3] == AccessType.unsafe_read) and
                (key1[3] not in set([AccessType.unsafe_read,
                                        AccessType.atomic_read,
                                        AccessType.mutex_read]))) or
               ((key1[3] == AccessType.atomic_read) and
                (key2[3] not in set([AccessType.unsafe_read,
                                        AccessType.atomic_read,
                                        AccessType.atomic_write,
                                        AccessType.mutex_read]))) or
               ((key2[3] == AccessType.atomic_read) and
                (key1[3] not in set([AccessType.unsafe_read,
                                        AccessType.atomic_read,
                                        AccessType.atomic_write,
                                        AccessType.mutex_read]))) or
               ((key1[3] == AccessType.atomic_write) and
                (key2[3] not in set([AccessType.atomic_read,
                                        AccessType.atomic_write]))) or
               ((key2[3] == AccessType.atomic_write) and
                (key1[3] not in set([AccessType.atomic_read,
                                        AccessType.atomic_write]))) or
               ((key1[3] == AccessType.mutex_read) and
                (key2[3] not in set([AccessType.unsafe_read,
                                        AccessType.mutex_read,
                                        AccessType.mutex_write,
                                        AccessType.atomic_read]))) or
               ((key2[3] == AccessType.mutex_read) and
                (key1[3] not in set([AccessType.unsafe_read,
                                        AccessType.mutex_read,
                                        AccessType.mutex_write,
                                        AccessType.atomic_read]))) or
               ((key1[3] == AccessType.mutex_write) and
                (key2[3] not in set([AccessType.mutex_read,
                                        AccessType.mutex_write]))) or
               ((key2[3] == AccessType.mutex_write) and
                (key1[3] not in set([AccessType.mutex_read,
                                     AccessType.mutex_write])))):
                    if((key1[4], key2[4]) not in races_lines_pairs):
                        races_lines_pairs.add((key1[4], key2[4]))
                        scalar_race_set.add((key1, key2))

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

def create_offset_span_label(parallel_id, lst):
    pp_id = offset_span_dict_pstart[parallel_id][0]
    osl = offset_span_dict_pstart[parallel_id][1]
    if(pp_id == 0):
        return [osl] + lst
    lst = [osl] + lst
    return create_offset_span_label(pp_id, lst)

AccessTypeName = ['None', 'Read', 'Write', 'Atomic Read', 'Atomic Write', 'Critical Read', 'Critical Write']
AccessTypeVar = ['_n', '_r', '_w', '_ar', '_aw', '_cr', '_cw']

PARALLEL_BREAK = "PARALLEL_BREAK"
PARALLEL_START = "PARALLEL_START"
PARALLEL_END = "PARALLEL_END"
DATA_BEGIN = "DATA_BEGIN"
DATA_END = "DATA_END"
DATA = "DATA"

files = sys.argv[1:-2]
directory = sys.argv[-2]
executable = sys.argv[-1]

for filename in files:
    # print "Reading file " + filename + "..."
    parallel_id = 0
    tid = 0;
    offset_span_label = ""
    # { parallel_id: (parent_parallel_id, offset_span)}
    offset_span_dict_pstart = {}
    offset_span_dict_pend = {}
    array_dict = defaultdict(lambda : defaultdict(list))
    scalar_dict = defaultdict(lambda : defaultdict(list))
    with open(directory + "/" + filename, "r") as f:
        for line in f:
            if(line.startswith(PARALLEL_BREAK)):
                break
            # parallel_id, parent_parallel_id, offset-span label [omp_tid:num_threads]
            elif(line.startswith(PARALLEL_START)):
                string = re.search(r"\[([A-Za-z0-9,:]+)\]", line)
                info = string.group(1).split(',')
                os = info[2].split(':')
                offset_span_dict_pstart[int(info[0])] = (int(info[1]), (int(os[0]), int(os[1])))
            elif(line.startswith(PARALLEL_END)):
                # string = re.search(r"\[([A-Za-z0-9,:]+)\]", line)
                # info = string.group(1).split(',')
                # os = info[2].split(':')
                # offset_span_dict_pend[int(info[0])] = (int(info[1]), (int(os[0]), int(os[1])))
                pass
            # parallel_id, ompt_tid, offset-span label [omp_tid:num_threads], barrier
            elif(line.startswith(DATA_BEGIN)):
                string = re.search(r"\[([A-Za-z0-9,:]+)\]", line)
                info = string.group(1).split(',')
                parallel_id = int(info[0])
                tid = int(info[1])
                os = info[2].split(':')
                oslabel = (int(os[0]), int(os[1]))
                barrier = int(info[3])
            elif(line.startswith(DATA_END)):
                pass
            # hash, address, count, size, type, pc
            elif(line.startswith(DATA)):
                string = re.search(r"\[([A-Za-z0-9,]+)\]", line)
                info = string.group(1).split(',')
                # label to identify if two threads are concurrent or not
                offset_span_label = create_offset_span_label(parallel_id, [oslabel])
                if(int(info[2]) > 1):
                    lst = info[1:]
                    lst.append(offset_span_label)
                    array_dict[info[0]][tid].extend(lst)
                    array_dict[info[0]][tid].append(barrier)
                else:
                    lst = info[1:2] + info[3:]
                    lst.append(offset_span_label)
                    scalar_dict[info[0]][tid].extend(lst)
                    scalar_dict[info[0]][tid].append(barrier)
    # print "Checking for races..."
    find_array_races(array_dict, filename)
    find_scalar_races(scalar_dict, filename)
