#!/usr/bin/python

import sys, os
from subprocess import Popen, PIPE
sys.path.append(os.environ['JENKINS_SCRIPTS_PATH'] + '/../shared/python')
from misc import getOptDict, printOutput
from traceback import format_exc

# --- BEGIN main program ---------------------------------------

# Validate command-line
options = getOptDict()
if not ('job' in options):
    sys.stderr.write('usage: ' + sys.argv[0] + ' --job <target job>\n')
    sys.exit(1)

# Execute job script
try:
    script = os.environ['JENKINS_SCRIPTS_PATH'] + '/jobs/' + options['job']
    p = Popen([script], stdout = PIPE, stderr = PIPE)
    stdout, stderr = p.communicate()
except:
    sys.stderr.write('failed to run job script {}:\n{}\n'.format(script, format_exc()))
    sys.exit(1)

if p.returncode == 0:
    # the job script terminated successfully
    printOutput(stdout, stderr)
    sys.exit(0)

# The job script failed, so notify relevant people
try:
    sys.stderr.write('the job script failed with exit code {}:\n'.format(p.returncode))
    printOutput(stdout, stderr)
    sys.stderr.write('notifying relevant people ...\n')
    cmd = [
        os.environ['JENKINS_SCRIPTS_PATH'] + '/notifybuildfailure.py',
        '--jenkinshome', os.environ['JENKINS_HOME'],
        '--jenkinsurl', os.environ['JENKINS_URL'],
        '--job', os.environ['JOB_NAME'],
        '--build', os.environ['BUILD_NUMBER']
        ]
    p = Popen(cmd, stdout = PIPE, stderr = PIPE)
    stdout, stderr = p.communicate()
    sys.stderr.write('notification succeeded\n')
except:
    sys.stderr.write('notification failed: {}\n'.format(format_exc()))
    sys.exit(1)

printOutput(stdout, stderr)

# Propagate failure
sys.exit(1)

# --- END main program ---------------------------------------
