#!/usr/bin/python

import sys, os, re
from subprocess import Popen, PIPE
#from misc import getOptDict
from misc import printOutput
from traceback import format_exc

"""
This script resets the virtual machine to be used for WMS testing.
"""

# --- BEGIN global functions ---------------------------------------
# --- END global functions ---------------------------------------


# --- BEGIN main program ---------------------------------------

# Validate command-line
#options = getOptDict()
#if not 'vm' in options:
#    sys.stderr.write('usage: ' + sys.argv[0] + ' --vm <virtual machine>\n')
#    sys.exit(1)

name = 'wms'
memory = 2048
cpus = 7
snapshot = 'snapshot1'

# Ensure VM is turned off
sys.stderr.write('ensure VM is powered off ... ')
cmd = 'vboxmanage controlvm {} poweroff'.format(name)
try:
    p = Popen(cmd.split(), stdout = PIPE, stderr = PIPE)
    stdout, stderr = p.communicate()
    sys.stderr.write('done\n')
except:
    sys.stderr.write('failed: {}\n'.format(format_exc()))
    sys.exit(1)
printOutput(stdout, stderr)
if p.returncode != 0:
    sys.stderr.write(
        ('ignoring non-zero exit code: {} (this would happen ' +
        'if the VM was already powered off, for example)\n').format(p.returncode))

# Restart VM from base snapshot
sys.stderr.write('restarting VM from base snapshot ... ')
cmd = '{}/vm_use {} {} {} {}'.format(os.path.dirname(os.path.abspath(__file__)), name, memory, cpus, snapshot)
try:
    p = Popen(cmd.split(), stdout = PIPE, stderr = PIPE)
    stdout, stderr = p.communicate()
    sys.stderr.write('done\n')
except:
    sys.stderr.write('failed: {}\n'.format(format_exc()))
    sys.exit(1)
printOutput(stdout, stderr)
if p.returncode != 0:
    sys.exit(1)

sys.exit(0)

# --- END main program ---------------------------------------
