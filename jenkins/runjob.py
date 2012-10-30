#!/usr/bin/python

import sys, os, re
from subprocess import Popen, PIPE
sys.path.append(os.environ['JENKINS_SCRIPTS_PATH'] + '/../shared/python')
from misc import getOptDict, printOutput
from traceback import format_exc
from xml.dom.minidom import parse

# --- BEGIN global functions ---------------------------------------

def finalExpectFailComment(comment):
    return '<no comment>' if len(comment.strip()) == 0 else comment

# --- END global functions ---------------------------------------


# --- BEGIN main program ---------------------------------------

# Validate command-line
options = getOptDict()
if not ('job' in options):
    sys.stderr.write(
        'usage: ' + sys.argv[0] + ' --job <target job> ' +
        '[--expectfail \'<comment about expected failure>\']\n')
    sys.exit(1)

# # Extract URL to current version of core build script
# try:
#     fname = '{}/jobs/scripts/lastSuccessful/build.xml'.format(os.environ['JENKINS_HOME'])
#     dom = parse(fname)
#     git_info_elem = dom.getElementsByTagName('hudson.plugins.git.util.BuildData')[0]
#     p = re.compile('^git://git.met.no/(.+)$')
#     m = p.match(git_info_elem.getElementsByTagName('remoteUrls')[0].getElementsByTagName(
#             'string')[0].childNodes[0].nodeValue)
#     repo = m.group(1)
#     fname = 'jenkins/jobs/{}'.format(options['job'])
#     sha1 = git_info_elem.getElementsByTagName('sha1')[0].childNodes[0].nodeValue
#     script_url = 'https://git.met.no/cgi-bin/gitweb.cgi?p={};a=blob;f={};hb={}'.format(repo, fname, sha1)
# except:
#     sys.stderr.write('warning: failed to get link to current version of core build script: {}\n'.format(
#             format_exc()))
#     script_url = 'failed to extract URL'
script_url = 'core build script URL unavailable for now'

# Execute core build script
script = os.environ['JENKINS_SCRIPTS_PATH'] + '/jobs/' + options['job']
sys.stderr.write('running core build script {} ({}) ...\n'.format(script, script_url))
try:
    p = Popen([script], stdout = PIPE, stderr = PIPE)
    bs_stdout, bs_stderr = p.communicate()
except:
    sys.stderr.write('failed to run core build script {}:\n{}\n'.format(script, format_exc()))
    sys.exit(1)

xpass = False
if p.returncode == 0:
    # the core build script terminated successfully
    if 'expectfail' in options:
        # the core build script was expected to fail, so mark incident to be reported
        xpass = True
    else:
        # the pass was expected, so we're done
        printOutput(bs_stdout, bs_stderr)
        sys.exit(0)

# At this point the core build script either 1) failed or 2) passed unexpectedly

cmd = [
    os.environ['JENKINS_SCRIPTS_PATH'] + '/notifybuildfailure.py',
    '--jenkinshome', os.environ['JENKINS_HOME'],
    '--jenkinsurl', os.environ['JENKINS_URL'],
    '--job', os.environ['JOB_NAME'],
    '--build', os.environ['BUILD_NUMBER']
    ]
if xpass:
    # unexpected pass
    comment = finalExpectFailComment(options['expectfail'])
    sys.stderr.write('error: the core build script ({}) passed unexpectedly: {}\n'.format(script, comment))
    cmd += ['--xpass', comment]
else:
    if 'expectfail' in options:
        # expected failure
        sys.stderr.write('the core build script ({}) failed (with exit code {}), but this was expected: {}\n'.format(
                script, p.returncode, finalExpectFailComment(options['expectfail'])))
        printOutput(bs_stdout, bs_stderr)
        sys.exit(0) # don't flag this as an error
    else:
        # unexpected (i.e. "normal") failure
        sys.stderr.write('error: the core build script ({}) failed with exit code {}\n'.format(script, p.returncode))

# At this point the pass or failure was unexpected

printOutput(bs_stdout, bs_stderr)
sys.stderr.write('notifying relevant people ...\n')

try:
    p = Popen(cmd, stdout = PIPE, stderr = PIPE)
    nf_stdout, nf_stderr = p.communicate()
    sys.stderr.write('notification succeeded\n')
except:
    sys.stderr.write('notification failed: {}\n'.format(format_exc()))
    sys.exit(1)

printOutput(nf_stdout, nf_stderr)

# Indicate that this script as such failed (since the core build script result was unexpected)
sys.exit(1)

# --- END main program ---------------------------------------
