#!/usr/bin/python

import numpy as np

a = np.zeros([5, 4])

print 'initial matrix:\n', a

v = 0
for i in range(a.shape[0]):
  for j in range(a.shape[1]):
     a[i, j] = v
     v = v + 1

print '\nmatrix after setting values:\n', a
