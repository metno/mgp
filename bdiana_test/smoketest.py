#!/usr/bin/python

# This program performs basic tests on the bdiana executable, like verifying
# that bdiana behaves as expected when launched without arguments.

import sys
from subprocess import Popen, PIPE

# Get command-line arguments
if len(sys.argv) != 2:
    sys.stderr.write('usage: ' + sys.argv[0] + ' <bdiana executable>\n')
    sys.exit(1)
bdiana = sys.argv[1]

# Execute bdiana without arguments and verify that the output makes sense
try:
    p = Popen([bdiana], stdout = PIPE, stderr = PIPE)
    stdout, stderr = p.communicate()
except OSError as e:
    sys.stderr.write('OS error({0}): {1}\n'.format(e.errno, e.strerror))
    sys.exit(1)
except:
    sys.stderr.write('Unexpected error: ' + sys.exc_info()[0] + '\n')
    sys.exit(1)

# For now just dump exit code and output. Eventually we want to compare against
# a baseline.
print 'return code:', p.returncode
print 'stdout:', stdout
print 'stderr:', stderr

sys.exit(0)
