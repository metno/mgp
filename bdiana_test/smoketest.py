#!/usr/bin/python

"""
This script verifies that the bdiana executable behaves in a sensible
way when launched without arguments.
The script exits with code 0 iff the test passes.
"""

import sys, os, re
from subprocess import Popen, PIPE
sys.path.append(os.environ['JENKINS_SCRIPTS_PATH'] + '/../shared/python')
from misc import getOptDict, printOutput

# --- BEGIN main program ---------------------------------------

# Validate command-line
options = getOptDict()
if not 'cmd' in options:
    sys.stderr.write(
        'usage: ' + sys.argv[0] + ' --cmd \'<bdiana command>\' ' +
        '[--expexit <expected exit code>] ' +
        '[--expout <file containing expected standard output> ' +
        '[--experr <file containing expected standard error>\n')
    sys.exit(1)

# Run bdiana command
try:
    leadmsg = 'Failed to run \'{}\''.format(options['cmd'])
    p = Popen(options['cmd'].split(), stdout = PIPE, stderr = PIPE)
    stdout, stderr = p.communicate()
except OSError as e:
    sys.stderr.write('{}: OS error({}): {}\n'.format(leadmsg, e.errno, e.strerror))
    sys.exit(1)
except:
    sys.stderr.write('{}: Unexpected error: {}\n'.format(leadmsg, sys.exc_info()[0]))
    sys.exit(1)

# Print exit code and output
sys.stderr.write('EXIT CODE: {}\n'.format(p.returncode))
printOutput(stdout, stderr)

# For now we don't implement detailed comparison of expected and actual output.
# Simply check that the word 'Usage:' occurs in stdout.
if stdout.find('Usage:') < 0:
    sys.stderr.write('Error: expected word \'Usage:\' not found in standard output\n')
    sys.exit(1)

# Smoke test passed
sys.stderr.write('Smoke test passed\n')
sys.exit(0)

# --- END main program ---------------------------------------
