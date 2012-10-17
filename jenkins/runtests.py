#!/usr/bin/python

"""
This script runs all tests in a test suite.
"""

import sys, os, re
from subprocess import Popen, PIPE
from misc import getOptDict, printOutput # Relies on PYTHONPATH being properly set
from traceback import format_exc
from xml.dom.minidom import parse

# --- BEGIN global functions ---------------------------------------

# Returns the 'insignificant' status of a test as a dictionary:
# { 'set': <boolean>, 'reason': <string> }
def insigStatus(cfg_path, fpath):
    # The test is considered insignificant iff all of the
    # following conditions are satisfied:
    # - a config file exists
    # - the config file contains an 'insignificant' element
    # - the 'insignificant' element does _not_ have a 'set' attribute that is false
    insig_set = False
    insig_reason = ''
    if fpath in cfg_fpath:
        dom = parse(cfg_path[fpath])
        insig_elems = dom.getElementsByTagName('insignificant')
        if len(insig_elems) > 0:
            insig_elem = insig_elems[0] # consider only the first one
            if ((not insig_elem.hasAttribute('set')) or
                (insig_elem.getAttribute('set').lower() not in ['0', 'false', 'no'])):
                insig_set = True
                insig_reason = '' if not insig_elem.firstChild else insig_elem.firstChild.nodeValue.strip()

    return { 'set': insig_set, 'reason': insig_reason }

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
nfail = nfail_insig = npass = npass_insig = 0
sys.stderr.write('\n{} TEST RESULTS {}\n'.format('='*6, '='*80))
for fpath in result:
    try:
        insig_stat = insigStatus(cfg_fpath, fpath)
    except:
        sys.stderr.write('failed to get \'insignificant\' status for \'{}\':\n{}\n'.format(
                fpath, format_exc()))
        sys.exit(1)

    sys.stderr.write('\n{}\n__TEST: {}\n'.format('_'*100, fpath))
    if insig_stat['set']:
        sys.stderr.write('__INSIGNIFICANT (failure is tolerated){}\n'.format(
                ':\n{}'.format(insig_stat['reason']) if insig_stat['reason'] != '' else ''))
    sys.stderr.write('\n')
    r = result[fpath]
    sys.stderr.write('__{}EXIT CODE: {}\n'.format('INSIGNIFICANT ' if insig_stat['set'] else '', r['exitcode']))
    if int(r['exitcode']) > 0:
        if insig_stat['set']:
            nfail_insig = nfail_insig + 1
        else:
            nfail = nfail + 1
    else:
        if insig_stat['set']:
            npass_insig = npass_insig + 1
        else:
            npass = npass + 1
    printOutput(r['stdout'], r['stderr'])


sys.stderr.write('\n{}\n'.format('='*100))
sys.stderr.write('           total tests: {}\n'.format(nfail + nfail_insig + npass + npass_insig))
sys.stderr.write('              failures: {}\n'.format(nfail))
sys.stderr.write('insignificant failures: {}\n'.format(nfail_insig))
sys.stderr.write('                passes: {}\n'.format(npass))
sys.stderr.write('  insignificant passes: {}\n'.format(npass_insig))
sys.stderr.write('flagging overall {}\n'.format('success' if nfail == 0 else 'failure'))

sys.exit(1 if nfail > 0 else 0) # the test suite fails iff at least one significant test fails

# --- END main program ---------------------------------------
