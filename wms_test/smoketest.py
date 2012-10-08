#!/usr/bin/python

"""
This script verifies that the WMS service responds to basic/minimal requests in a sensible way
The script exits with code 0 iff the test passes.
"""

import sys, os
from subprocess import Popen, PIPE
sys.path.append(os.environ['JENKINS_SCRIPTS_PATH'] + '/../shared/python')
from misc import getOptDict, printOutput
# The next line is a workaround. Check http://lxml.de/installation.html for
# how to install lxml properly.
sys.path.append('/home/joa/lib/python2.7/site-packages/lxml-2.3.6-py2.7-linux-x86_64.egg')
from lxml import etree
from traceback import format_exc

# --- BEGIN global functions ---------------------------------------

# Downloads and returns a document from an URL.
def download(url):
    sys.stdout.write('downloading capabilities document from {} ...\n'.format(url))
    cmd = ['wget', '-O', '-', url]
    p = Popen(cmd, stdout = PIPE, stderr = PIPE)
    stdout, stderr = p.communicate()
    sys.stdout.write(
        'exit code: {}; stdout: {} bytes; stderr: {} bytes\n'.format(p.returncode, len(stdout), len(stderr)))
    if p.returncode != 0:
        sys.stderr.write('error: {} failed with exit code {}:\n'.format(' '.join(cmd), p.returncode))
        printOutput(stdout, stderr)
        raise Exception

    return stdout


# Validates a capabilities document against an XML Schema.
# Raises an exception with an appropriate message iff the validation fails.
def validateCapabilities(cap_str, schema_fname):
    # Parse capabilities document
    try:
        cap_doc = etree.fromstring(cap_str)
    except:
        raise Exception, 'failed to parse capabilities document: {}'.format(format_exc())

    # Parse schema document
    try:
        schema_doc = etree.parse(schema_fname)
    except:
        raise Exception, 'failed to parse schema {}: {}'.format(schema_fname, format_exc())

    # Create schema
    try:
        schema = etree.XMLSchema(schema_doc)
    except:
        raise Exception, 'failed to create schema object from {}: {}'.format(
            schema_fname, format_exc())

    # Validate capabilities document against schema
    try:
        schema.assertValid(cap_doc)
    except etree.DocumentInvalid as e:
        #sys.stderr.write('error log: {}\n'.format(str(schema.error_log)))
        raise Exception, 'capabilities document is invalid according to XML Schema {}: {}'.format(
            schema_fname, e)
    except:
        raise Exception, 'other exception: {}'.format(format_exc())


# --- END global functions ---------------------------------------


# --- BEGIN main program ---------------------------------------

# Validate command-line
options = getOptDict()
if not 'schema' in options:
    sys.stderr.write(
        'usage: ' + sys.argv[0] + ' --schema <XML Schema file>\n')
    sys.exit(1)

base_url = 'http://c1wms2.met.no/verportal/verportal.map?service=WMS&version=1.3.0' # For now

# Download capabilities document
try:
    url = base_url +'&request=GetCapabilities'
    cap = download(url)
except Exception:
    sys.stderr.write('error: failed to download capabilities document from {}: {}\n'.format(url, format_exc()))
    sys.exit(1)

# Validate capabilities
try:
    validateCapabilities(cap, options['schema'])
except Exception:
    sys.stderr.write('error: failed to validate capabilities document: {}\n'.format(format_exc()))
    sys.exit(1)

sys.stderr.write('smoke test passed\n')
sys.exit(0)

# --- END main program ---------------------------------------
