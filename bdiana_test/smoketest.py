#!/usr/bin/python

"""
This script verifies that the bdiana executable behaves in a sensible
way when launched without arguments.
The script indicates the test result by writing 'pass' or 'fail' to stdout
(all other output is written to stderr).
"""

import sys, re
from subprocess import Popen, PIPE
sys.path.append('../shared/python')
from misc import getOptDict

# --- BEGIN global functions ---------------------------------------

# Exits script with status = fail.
def _fail():
    sys.stdout.write('fail\n')
    sys.exit(0)

# Exits script with status = pass.
def _pass():
    sys.stdout.write('pass\n')
    sys.exit(0)

# --- END global functions ---------------------------------------


# --- BEGIN main program ---------------------------------------

# Validate command-line
options = getOptDict()
if not 'cmd' in options:
    sys.stderr.write(
        'usage: ' + sys.argv[0] + ' --cmd \'<bdiana command>\' ' +
        '[--expexit <expected exit code>] ' +
        '[--expout <file containing expected standard output> ' +
        '[--experr <file containing expected standard error>\n')
    _fail()

# Run bdiana command
try:
    leadmsg = 'Failed to run \'{}\''.format(options['cmd'])
    p = Popen(options['cmd'].split(), stdout = PIPE, stderr = PIPE)
    stdout, stderr = p.communicate()
except OSError as e:
    sys.stderr.write('{}: OS error({}): {}\n'.format(leadmsg, e.errno, e.strerror))
    _fail()
except:
    sys.stderr.write('{}: Unexpected error: {}\n'.format(leadmsg, sys.exc_info()[0]))
    _fail()

# Print exit code and output
sys.stderr.write('EXIT CODE: {}\n'.format(p.returncode))
w = 40
sys.stderr.write(
    '--- BEGIN standard output ' + '-'*w +
    '\n{}--- END standard output '.format(stdout) + '-'*w + '\n')
sys.stderr.write(
    '--- BEGIN standard error ' + '-'*w +
    '\n{}--- END standard error '.format(stderr) + '-'*w + '\n')

# For now we don't implement detailed comparison of expected and actual output.
# Simply check that the word 'Usage:' occurs in stdout.
if stdout.find('Usage:') < 0:
    sys.stderr.write('Error: expected word \'Usage:\' not found in standard output\n')
    _fail()

# Smoke test passed
sys.stderr.write('Smoke test passed\n')
_pass()

# --- END main program ---------------------------------------
