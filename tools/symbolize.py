#!/usr/bin/python

import re
import sys
import subprocess
from operator import itemgetter

if(len(sys.argv)) < 3:
    print "Specify input file and executable!\n"
    sys.exit(0)

filename = sys.argv[1]
executable = sys.argv[2]

g = open(filename + '_addr.txt', 'w')
with open(filename, "r") as f:
    for line in f:
        if(line):
            addresses = re.findall(r'0x[0-9A-Fa-f]+', line, re.I)
            for address in addresses:
                fileline = executable + ' ' + address + '\n'
                g.write(fileline)
g.close()

symbolizer = 'which llvm-symbolizer'
process = subprocess.Popen(symbolizer.split(), stdout=subprocess.PIPE)
path = process.communicate()
command = path[0].rstrip() + ' -pretty-print' + ' < ' + filename + '_addr.txt'
proc = subprocess.Popen(command, shell=True,
                        stdin=subprocess.PIPE,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.STDOUT)

races = []
while True:
    line = proc.stdout.readline()
    if line != '':
        if(line != '\n'):
            # l = line.rsplit('\n')[0].rsplit()[2].split(':')
            # races.append([l[0], int(l[1], 10), int(l[2], 10)])
            # races.append(line.rsplit('\n')[0].rsplit()[2])
            l = line.rsplit('\n')[0].rsplit()[2].split(':')
            if(len(l) > 1):
                races.append(l[0] + ":" + l[1])
    else:
        break;

races = list(set(races))
new_races = []
for race in races:
    r = race.split(':')
    if(len(r) > 1):
        new_races.append([r[0], int(r[1], 10)]) #, int(r[2], 10)])
new_races.sort(key = itemgetter(0,1)) # ,2))
race_list = []
for race in new_races:
    race_list.append(race[0] + ':' + str(race[1])) # + ':' + str(race[2]))

print "ARCHER RACES:"
print '\n'.join(race_list)
print
print "ERRORS:"
print proc.stderr