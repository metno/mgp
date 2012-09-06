#!/usr/bin/python

"""
This script verifies that the bdiana executable behaves in a sensible
way when launched without arguments.
"""

import sys, re
from subprocess import Popen, PIPE

# --- BEGIN global functions ---------------------------------------

# Extracts the option dictionary from the command line
def getOptDict():
    options = {}
    p = re.compile('^--(.+)$')
    key = None
    for arg in sys.argv[1:]:
        if key != None:
            options[key] = arg
        m = p.match(arg)
        if m:
            key = m.group(1)
            # Support '-help' as the only value-less option:
            if key == 'help':
                options[key] = 1
                key = None
        else:
            key = None
    return options

# --- END global functions ---------------------------------------


# --- BEGIN main program ---------------------------------------

# Extract command-line options
options = getOptDict()
if not 'exec' in options:
    sys.stderr.write(
        'usage: ' + sys.argv[0] + ' --exec <bdiana executable> ' +
        '[--expexit <expected exit code>] ' +
        '[--expout <file containing expected standard output> ' +
        '[--experr <file containing expected standard error>\n')
    sys.exit(1)
bdiana_exec = options['exec']

# Run executable without arguments
try:
    leadmsg = 'Failed to run {}'.format(bdiana_exec)
    p = Popen([bdiana_exec], stdout = PIPE, stderr = PIPE)
    stdout, stderr = p.communicate()
except OSError as e:
    sys.stderr.write('{}: OS error({}): {}\n'.format(leadmsg, e.errno, e.strerror))
    sys.exit(1)
except:
    sys.stderr.write('{}: Unexpected error: {}\n'.format(leadmsg, sys.exc_info()[0]))
    sys.exit(1)

# Print exit code and output
sys.stderr.write('EXIT CODE: {}\n'.format(p.returncode))
w = 40
sys.stderr.write(
    '--- BEGIN standard output ' + '-'*w +
    '\n{}\n--- END standard output '.format(stdout) + '-'*w + '\n')
sys.stderr.write(
    '--- BEGIN standard error ' + '-'*w +
    '\n{}\n--- END standard error '.format(stderr) + '-'*w + '\n')

# For now we don't implement detailed comparison of expected and actual output.
# Simply check that the word 'Usage:' occurs in stdout.
if stdout.find('Usage:') < 0:
    sys.stderr.write('Error: expected word \'Usage:\' not found in standard output\n')
    sys.exit(1)

# Smoke test passed
sys.stderr.write('Smoke test passed\n')
sys.exit(0)

# --- END main program ---------------------------------------
