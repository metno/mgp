#!/usr/bin/python

"""
This script runs all tests in a test suite.
"""

import sys, os, re
from subprocess import Popen, PIPE
from misc import getOptDict, printOutput # Relies on PYTHONPATH being properly set
from traceback import format_exc

# --- BEGIN global functions ---------------------------------------
# --- END global functions ---------------------------------------

# --- BEGIN main program ---------------------------------------

# Validate command-line
options = getOptDict()
if not 'dir' in options:
    sys.stderr.write(
        'usage: ' + sys.argv[0] + ' --dir <subdirectory containing tests>\n')
    sys.exit(1)
if not os.path.isdir(options['dir']):
    sys.stderr.write('\'{}\' is not a directory\n'.format(options['dir']))
    sys.exit(1)


# Collect tests and configurations
tst_fpaths = [] # List of test files (executable files of the form *.tst)
cfg_fpath = {} # Test file to config file mapping
               # (note: a config file needs to live in the same dir as the test!)
tst_file_ptn = re.compile('^(\S+).tst$')
for dirpath, dirnames, filenames in os.walk(options['dir']):
    for fname in filenames:
        m = tst_file_ptn.match(fname)
        if m:
            fpath = '{}/{}'.format(dirpath, fname)
            if os.access(fpath, os.X_OK):
                tst_fpaths.append(fpath)
                basename = m.group(1)
                cfpath = '{}/{}.cfg'.format(dirpath, basename)
                if os.path.isfile(cfpath):
                    cfg_fpath[fpath] = cfpath


# Run tests
result = {}
for fpath in tst_fpaths:
    sys.stderr.write('running test {} ... '.format(fpath))
    try:
        p = Popen([fpath], stdout = PIPE, stderr = PIPE)
        stdout, stderr = p.communicate()
    except:
        sys.stderr.write('failed:\n{}\n'.format(format_exc()))
        sys.exit(1)
    sys.stderr.write('done\n')
    result[fpath] = { 'exitcode': p.returncode, 'stdout': stdout, 'stderr': stderr }

# Present results
nfail = npass = 0
sys.stderr.write('\n{} TEST RESULTS {}\n'.format('='*6, '='*80))
for fpath in result:
    sys.stderr.write('\n{}\n__TEST: {}\n\n'.format('_'*100, fpath))
    r = result[fpath]
    sys.stderr.write('__EXIT CODE: {}\n'.format(r['exitcode']))
    if int(r['exitcode']) > 0:
        nfail = nfail + 1
    else:
        npass = npass + 1
    printOutput(r['stdout'], r['stderr'])

sys.stderr.write('\n{}\n{} out of {} tests failed - flagging overall {}\n'.format(
        '='*100, nfail, nfail + npass, 'success' if nfail == 0 else 'failure'))
sys.exit(1 if nfail > 0 else 0) # the test suite fails iff at least one individual test fails

# --- END main program ---------------------------------------
