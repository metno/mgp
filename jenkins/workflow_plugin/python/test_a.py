#!/usr/bin/python

import sys, os
from time import sleep

sys.stderr.write('test_a.py running; pid:{} ...\n'.format(os.getpid()))

try:
    n = int(sys.argv[1])
    s = float(sys.argv[2])
except:
    sys.stderr.write('usage: {} <n iterations> <secs to sleep in each iteration>\n'.format(sys.argv[0]))
    sys.exit(1)

for i in range(n):
  sys.stderr.write('iteration {}:{}; sleeping {} secs ...\n'.format(i + 1, n, s))
  sleep(s)

sys.exit(0)
