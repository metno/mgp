#!/usr/bin/python

import sys, os
from time import sleep

try:
    n = int(sys.argv[1])
    s = float(sys.argv[2])
except:
    sys.stderr.write('\
usage: {} <n iterations> <secs to sleep in each iteration> \
[<iteration after which to fail with exit code 1 (1 = after first iteration etc.)>]\n'.format(
            sys.argv[0]))
    sys.exit(1)

sys.stderr.write('test_a.py running; pid:{} ...\n'.format(os.getpid()))

try:
    fail_iter = int(sys.argv[3]) - 1
except:
    fail_iter = -1 # default, i.e. never fail

for i in range(n):
  sys.stderr.write('iteration {}:{}; sleeping {} secs ...\n'.format(i + 1, n, s))
  sleep(s)
  if i == fail_iter:
      sys.exit(17)

sys.exit(0)
