#!/usr/bin/python

"""
This script verifies that the WMS service responds to basic/minimal requests in a sensible way
The script exits with code 0 iff the test passes.
"""

import sys, os
from subprocess import Popen, PIPE
sys.path.append(os.environ['JENKINS_SCRIPTS_PATH'] + '/../shared/python')
from misc import printOutput
from traceback import format_exc
from xml.dom.minidom import parseString

# --- BEGIN global functions ---------------------------------------

# Verifies (by not raising an exception) that expressions e1 and e2 are equal.
def verifyEqual(e1, e2):
    if e1 != e2:
        raise Exception, '{} != {}'.format(e1, e2)


# Verifies (by not raising an exception) that we're able to get a sensible capabilities document.
def getCapabilities():

    # Download the capabilities document
    cmd = [
        'wget', '-O', '-',
        'http://c1wms2.met.no/verportal/verportal.map?service=WMS&request=GetCapabilities&version=1.3.0']
    p = Popen(cmd, stdout = PIPE, stderr = PIPE)
    stdout, stderr = p.communicate()
    sys.stdout.write(
        'exit code: {}; stdout: {} bytes; stderr: {} bytes\n'.format(p.returncode, len(stdout), len(stderr)))
    if p.returncode != 0:
        sys.stderr.write('error: {} failed with exit code {}:\n'.format(' '.join(cmd), p.returncode))
        printOutput(stdout, stderr)
        raise Exception

    # Check that the document contains sensible information
    dom = parseString(stdout)
    verifyEqual(len(dom.getElementsByTagName('Service')), 1)
    verifyEqual(len(dom.getElementsByTagName('Capability')), 1)

# --- END global functions ---------------------------------------


# --- BEGIN main program ---------------------------------------

try:
    getCapabilities()
except Exception:
    sys.stderr.write('getCapabilities() failed: {}\n'.format(format_exc()))
    sys.exit(1)

sys.stderr.write('Smoke test passed\n')
sys.exit(0)

# --- END main program ---------------------------------------
