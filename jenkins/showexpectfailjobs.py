#!/usr/bin/python

"""
This script lists Jenkins jobs for which the --expectfail option was passed to runjob.py.
"""

import sys, os, re
sys.path.append(os.environ['JENKINS_SCRIPTS_PATH'] + '/../shared/python')
from misc import getOptDict
from xml.dom.minidom import parse
from traceback import format_exc

# --- BEGIN main program ---------------------------------------

# Validate command-line
options = getOptDict()
if not 'jenkinshome' in options:
    sys.stderr.write('usage: ' + sys.argv[0] + ' --jenkinshome <Jenkins home directory>\n')
    sys.exit(1)

jobs_root_dir = '{}/jobs'.format(options['jenkinshome'])
jobs = [
    name for name in os.listdir(jobs_root_dir)
    if os.path.isdir(os.path.join(jobs_root_dir, name))]

expectfail = {}
expectpass = {}
error = {}
for job in jobs:
    fname = '{}/config.xml'.format(os.path.join(jobs_root_dir, job))
    try:
        dom = parse(fname)
        command_elems = dom.getElementsByTagName('command')
        if len(command_elems) == 0:
            error[job] = 'no \'command\' element found in {}'.format(fname)
        elif len(command_elems[0].childNodes) == 0:
            expectpass[job] = ''
        else:
            command = command_elems[0].childNodes[0].nodeValue
            if '--expectfail' in command:
                expectfail[job] = command
            else:
                expectpass[job] = command
    except:
        error[job] = format_exc()

w = 100
group_line = '='*w + '\n'
item_line = '_'*w + '\n'

sys.stdout.write(group_line)
sys.stdout.write('*** Jobs expected to fail ({}):\n'.format(len(expectfail)))
for job in expectfail:
    sys.stdout.write('{}   Name: {}\nCommand: {}\n'.format(item_line, job, expectfail[job]))

sys.stdout.write('\n' + group_line)
sys.stdout.write('*** Jobs expected to pass ({}):\n'.format(len(expectpass)))
for job in expectpass:
    sys.stdout.write('{}   Name: {}\nCommand: {}\n'.format(item_line, job, expectpass[job]))

sys.stdout.write('\n' + group_line)
sys.stdout.write('*** Problematic jobs ({}):\n'.format(len(error)))
for job in error:
    sys.stdout.write('{} Name: {}\nError: {}\n'.format(item_line, job, error[job]))

# --- END main program ---------------------------------------
