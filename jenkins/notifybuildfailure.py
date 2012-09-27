#!/usr/bin/python

"""
This script is intended to be called from a Jenkins job whenever the job fails.
The following people are notified by email about the job failure:

  1: those who have explicitly registered to be notified about
     failures of this job, and

  2: those who have committed changes to a source repository (local or
     upstream) since the last successful job build (the idea being
     that one of these changes caused the job to fail).

Assumptions:

 - At most one git repo per job is supported for now (need to
   figure out the exact XML format).

 - Email aliases are not supported. Although the 'met.no' domain is
   recognized, 'joa' (svn) (or 'joa@met.no' (git)) would be considered
   different from 'jo.asplin@met.no' (git).
"""

import sys, os, re
sys.path.append(os.environ['JENKINS_SCRIPTS_PATH'] + '/../shared/python')
from misc import getOptDict
from xml.dom.minidom import parse, parseString
from subprocess import Popen, PIPE
from datetime import datetime
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from traceback import format_exc

# --- BEGIN classes ---------------------------------------

# Base class for extracting information that depends on the version control system (git, SVN ...).
# This class employs the template method pattern.
class VCSExtractor(object):
    # Returns the URL for the description of a repository revision.
    def revisionURL(self, repo_url, rev):
        m = re.search(self.repoNamePattern(), repo_url)
        if m:
            repo_name = m.group(1)
        else:
            raise Exception('failed to extract repo name from {}'.format(repo_url))
        url = self.revURLTemplate().format(repo_name, rev)
        return url

    # For the given job and source repository, this function extracts descriptions of all
    # commits with revision number later than 'last_pass_rev'. The description records are added
    # to the dictionary 'commits'.
    def addCandidateCommits(self, jenkins_home, job, repo_url, last_pass_rev, commits):
        hack = False
        if hack:
            print '### hack to get more revs'
        self.offset = 3 if hack else 0
        workspace = '{}/jobs/{}/workspace'.format(jenkins_home, job)
        self.addCandidateCommits_helper(workspace, repo_url, last_pass_rev, commits)
        for commit in commits:
            commit['vcs_ext'] = self

    def emailAddress(author):
        return self.emailAddress_helper(author)


class GitExtractor(VCSExtractor):
    @staticmethod
    def repoNamePattern():
        return '^git://git.met.no/(.+)$'

    @staticmethod
    def revURLTemplate():
        return 'https://git.met.no/cgi-bin/gitweb.cgi?p={};a=commit;h={}'

    def addCandidateCommits_helper(self, workspace, repo_url, last_pass_rev, commits):
        os.chdir(workspace)
        cmd = [
            'git', 'log', '{}~{}..HEAD'.format(last_pass_rev, self.offset), '--pretty=format:%H %ce %ct %s']
        p = Popen(cmd, stdout = PIPE, stderr = PIPE)
        stdout, stderr = p.communicate()

        if p.returncode != 0:
            raise Exception('git command \'{}\' failed in workspace \'{}\''.format(' '.join(cmd), workspace))

        # Do nothing if stdout is empty (this typically means that no revisions exist
        # after last_pass_rev)
        if len(stdout) == 0:
            return

        lines = filter(lambda line: len(line.strip()) > 0, stdout.split('\n'))
        for line in lines:
            words = line.split()
            rev = words[0]
            author = words[1]
            date = datetime.fromtimestamp(int(words[2]))
            msg = ' '.join(words[3:])
            commits.append( { 'rev': rev, 'author': author, 'date': date, 'msg': msg } )

    @staticmethod
    def emailAddress(author):
        return author


class SVNExtractor(VCSExtractor):
    @staticmethod
    def repoNamePattern():
        return '^https://svn.met.no/([^/]+)'

    @staticmethod
    def revURLTemplate():
        return 'https://svn.met.no/viewvc/{}?view=revision&revision={}'

    def addCandidateCommits_helper(self, workspace, repo_url, last_pass_rev, commits):
        cmd = 'svn log {} -rHEAD:{} --xml --non-interactive --trust-server-cert'.format(
            repo_url, int(last_pass_rev) + 1 - self.offset)
        p = Popen(cmd.split(), stdout = PIPE, stderr = PIPE)
        stdout, stderr = p.communicate()

        # Do nothing if the return code is non-zero (this typically means that no revisions exist
        # after last_pass_rev)
        if p.returncode != 0:
            return

        dom = parseString(stdout)
        for log_entry in dom.getElementsByTagName('logentry'):
            rev = log_entry.getAttribute('revision')
            author = log_entry.getElementsByTagName('author')[0].childNodes[0].nodeValue
            date = datetime.strptime(
                log_entry.getElementsByTagName('date')[0].childNodes[0].nodeValue,
                '%Y-%m-%dT%H:%M:%S.%fZ')
            msg = log_entry.getElementsByTagName('msg')[0].childNodes[0].nodeValue
            commits.append( { 'rev': rev, 'author': author, 'date': date, 'msg': msg } )

    @staticmethod
    def emailAddress(author):
        return author + '@met.no'

# --- END classes ---------------------------------------


# --- BEGIN global functions ---------------------------------------

# Adds information from a build status file into two structures:
#     1: revs: a list of dictionaries of (repository, revision) combinations.
#     2: build: a dictionary of (job, build number) combinations.
def addRevsAndBuildNumbers(fname, job, git_ext, svn_ext, revs, build):
    dom = parse(fname)
    rev = {}

    # Add SVN repos
    svn_info_elems = dom.getElementsByTagName('hudson.scm.SVNRevisionState')
    if len(svn_info_elems) > 0:
        svn_repo_elems = svn_info_elems[0].getElementsByTagName('entry')
        for repo_elem in svn_repo_elems:
            rev[repo_elem.getElementsByTagName('string')[0].childNodes[0].nodeValue] = \
                { 'value': repo_elem.getElementsByTagName('long')[0].childNodes[0].nodeValue,
                  'vcs_ext': svn_ext }

    # Add git repos
    git_info_elems = dom.getElementsByTagName('hudson.plugins.git.util.BuildData')
    if len(git_info_elems) > 0:
        git_info_elem = git_info_elems[0] # ### Multiple git repos not supported for now
        rev[git_info_elem.getElementsByTagName('remoteUrls')[0].getElementsByTagName(
                'string')[0].childNodes[0].nodeValue] = \
                { 'value': git_info_elem.getElementsByTagName('sha1')[0].childNodes[0].nodeValue,
                  'vcs_ext': git_ext }

    revs.append( { 'job': job, 'rev': rev } )

    # Add build number
    build[job] = dom.getElementsByTagName('number')[0].childNodes[0].nodeValue


# For the builds and source repos that could affect a target job
# (i.e. local or upstream ones), this function extracts info
# associated with the last successful build (if any) of the target
# job.
#
# If a last successful build exists, the following structures are populated:
#   1: revs: A list of dictionaries, one for each job that contains source repos.
#      Each dictionary contains the job name and a dictionary of
#      (repo URL, revision number) combinations.
#   2: last_pass_build: A dictionary of (job, last pass build number) combinations.
#
def getLastPassInfo(jenkins_home, tgt_job, revs, last_pass_build):
    # Clear out parameters
    del revs[:]
    last_pass_build.clear()

    git_ext = GitExtractor()
    svn_ext = SVNExtractor()

    # STEP 1: Get the revision number for each local repo:
    local_fname = '{}/jobs/{}/lastSuccessful/build.xml'.format(jenkins_home, tgt_job)
    if not os.path.exists(local_fname):
        return # Apparently the target job has never succeeded
    addRevsAndBuildNumbers(local_fname, tgt_job, git_ext, svn_ext, revs, last_pass_build)

    # STEP 2: For all upstream jobs, get the
    # ... relevant job/build combinations:
    dom = parse(local_fname)
    upstream_job_elems = dom.getElementsByTagName('upstreamProject')
    upstream_build_elems = dom.getElementsByTagName('upstreamBuild')
    upstream_builds = []
    for i in range(len(upstream_job_elems)):
        upstream_builds.append({
                'job': upstream_job_elems[i].childNodes[0].nodeValue,
                'build': upstream_build_elems[i].childNodes[0].nodeValue
                })
    # ... and then the repo/rev combinations:
    for upstream_build in upstream_builds:
        fname = '{}/jobs/{}/builds/{}/build.xml'.format(
            jenkins_home, upstream_build['job'], upstream_build['build'])
        addRevsAndBuildNumbers(fname, upstream_build['job'], git_ext, svn_ext, revs, last_pass_build)
        assert last_pass_build[upstream_build['job']] == upstream_build['build']


# Returns the list of email addresses of the people registered to be
# notified whenever a target job fails.
def getFixedRecipients(jenkins_home, tgt_job):
    fname = '{}/jobs/{}/config.xml'.format(jenkins_home, tgt_job)
    dom = parse(fname)
    recp_elem = dom.getElementsByTagName('recipients')[0]
    recipients = recp_elem.childNodes[0].nodeValue.split()
    return recipients


# Returns the Jenkins admin email address.
def getJenkinsAdminAddr(jenkins_home):
    fname = '{}/hudson.tasks.Mailer.xml'.format(jenkins_home)
    dom = parse(fname)
    addr_elem = dom.getElementsByTagName('adminAddress')[0]
    return addr_elem.childNodes[0].nodeValue


# Sends a single email.
def sendEmail(from_addr, to_addr, subject, html):
    # Create message container - the correct MIME type is multipart/alternative.
    msg = MIMEMultipart('alternative')
    msg['Subject'] = subject
    msg['From'] = from_addr
    msg['To'] = to_addr
    text = "Warning: plain text not supported - please enable HTML in the email client"

    # Record the MIME types of both parts - text/plain and text/html.
    part1 = MIMEText(text, 'plain')
    part2 = MIMEText(html, 'html')

    # Attach parts into message container.
    # According to RFC 2046, the last part of a multipart message, in this case
    # the HTML message, is best and preferred.
    #msg.attach(part1)
    msg.attach(part2)

    # Send the message via local SMTP server.
    s = smtplib.SMTP('localhost')
    # sendmail function takes 3 arguments: sender's address, recipient's address
    # and message to send - here it is sent as one string.
    s.sendmail(from_addr, to_addr, msg.as_string())
    s.quit()


# Sends an email to the following people:
#   - fixed recipients, i.e. those who have explicitly registered to
#     be notified about build failures of the target job
#   - candidate committers, i.e. people who may potentially have
#     caused the current failure.
#
# cand_commits is a list of dictionaries, one for each source job
#     (local or upstream).  Each job dictionary contains the job name
#     and a dictionary of source repositories for this job. Each repo
#     dictionary contains the revision number associated with the last
#     sucessful target job and the list of dictionaries for the
#     commits made after this point.  Each commit dictionary contains
#     the repo URL as key and a dictionary containing the description
#     for an individual commit as value: { 'rev': ..., 'author': ...,
#     'date': ..., 'msg': ... }.
#
def sendEmails(
    fixed_recipients, cand_commits, jenkins_url, tgt_job, tgt_build, last_pass_build, from_addr, xpass_msg,
    report):
    # Create top part of html document
    html_top = '<html>'
    html_top += """
        <head>
        <style type="text/css">
        table { border:1px solid; border-collapse:collapse }
        th { border:1px solid; border-collapse:collapse; border-color:#888; padding:0px 10px;
          background:#ddd; text-align:left }
        td { border:1px solid; border-collapse:collapse; border-color:#888; padding:0px 10px;
           vertical-align:top }
        .descr { font-family: monospace, "Lucida Console", "Courier New"; font-size: 95%;
           color:#000 }
        .hilight { background:#ffa }
        </style>
        </head>
    """
    html_top += '<body>'

    emails = set()

    # Create bottom part of html document
    html_bot_tmpl = ''
    for job_info in cand_commits:

        job = job_info['job']
        html_bot_tmpl += """
            <br/><hr><span style="font-size: 120%">Source repositories {} <b>{}</b>
            (build associated with last successful build of <b>{}</b>: <a href="{}">#{}</a>):</span>
            <br/>
        """.format(
            'local to' if (job == tgt_job) else 'for upstream job', job, tgt_job,
            '{}/job/{}/{}/'.format(jenkins_url, job, last_pass_build[job]),
            last_pass_build[job])

        repo_info = job_info['repo_info']
        for repo_url in repo_info:
            html_bot_tmpl += """
                <br/>Candidate changes in {} (i.e. later than Rev. {}):
            """.format(
                repo_url, repo_info[repo_url]['last_pass_rev'])
            commits = repo_info[repo_url]['commits']
            if len(commits) == 0:
                html_bot_tmpl += '<span style="color:red"><b>none</b></span><br/>'
            else:
                html_bot_tmpl += '<br/><table>'
                html_bot_tmpl += '<tr><th>Rev.</th><th>Author</th><th>Date</th><th>Description</th></tr>'
                for commit in commits:
                    email = commit['vcs_ext'].emailAddress(commit['author'])
                    emails.add(email)
                    html_bot_tmpl += (
                        '<tr class="_x_{}_x_"><td><a href={}>{}</a></td><td>{}</td>' +
                        '<td style="white-space: nowrap;">{}</td><td class=descr>{}</td></tr>').format(
                        email, commit['vcs_ext'].revisionURL(repo_url, commit['rev']), commit['rev'], email,
                        commit['date'].strftime('%Y-%m-%d %H:%M:%S'), commit['msg'])
                html_bot_tmpl += '</table>'

    html_bot_tmpl += '</body>'
    html_bot_tmpl += '</html>'

    tgt_url = '{}/job/{}/{}/console'.format(jenkins_url, tgt_job, tgt_build)
    if len(last_pass_build) > 0:
        tgt_last_pass_url = '{}/job/{}/{}/'.format(jenkins_url, tgt_job, last_pass_build[tgt_job])
    if xpass_msg != None:
        incident = '<span style="color:#f00">the core build script <u>passed unexpectedly</u></span>: ' + xpass_msg
    else:
        incident = '<span style="color:#f00">the core build script <u>failed</u></span>'
    report['cand_committers'] = []
    report['fixed_recipients'] = []

    # Notify candidate committers (NOTE: email aliases are not supported)
    for email in emails:
        p = re.compile('_x_{}_x_'.format(email))
        html_bot = p.sub('hilight', html_bot_tmpl) # highlight commits associated with this email address
        html_mid = """
            You receive this email because you may potentially have caused
            the failure of <a href="{}"><b>{}</b> #{}</a>.
            <br/><br/>
            Incident: {}
            <br/><br/>
            Changes in local and upstream repositories since the <a href="{}">last
            successful build (#{})</a> are listed below.
            <br/><br/>
            Please investigate.
            <br/><br/>
            <span style="background-color: #eee; font-size:
            80%"><b>Note:</b> This email was sent to {}. Email aliases
            are not supported yet, so a given user will receive a
            separate email for each syntactically different email
            address deduced from the affected source repositories
            (e.g. foob@met.no and foo.bar@met.no would be considered
            different even if they refer to the same person).
            </span><br/>
        """.format(tgt_url, tgt_job, tgt_build, incident, tgt_last_pass_url, last_pass_build[tgt_job], email)
        html = html_top + html_mid + html_bot

        to_addr = email
        sendEmail(from_addr, to_addr,
                  'Jenkins alert: {} #{} failed - please investigate'.format(tgt_job, tgt_build), html) ###
        report['cand_committers'].append(to_addr)

    # Notify fixed recipients
    for recp in fixed_recipients:
        html_mid = """
            You receive this email because you have registered to be notified whenever
            <b>{}</b> fails (in this case <a href="{}">Build #{}</a>).
            <br/><br/>
            Incident: {}
            <br/><br/>{}
            <br/>
        """.format(
            tgt_job, tgt_url, tgt_build, incident,
            """
            Changes in local and upstream repositories since the <a href="{}">last
            successful build (#{})</a> are listed below.<br/><br/>
            Candidate committers have been notified separately.
            """.format(tgt_last_pass_url, last_pass_build[tgt_job]) if len(last_pass_build) > 0 else
            """
            <b>Note:</b> This job has never built successfully.
            """)
        html = html_top + html_mid + html_bot_tmpl
        sendEmail(from_addr, recp, 'Jenkins alert: {} #{} failed'.format(tgt_job, tgt_build), html) ###
        report['fixed_recipients'].append(recp)

# --- END global functions ---------------------------------------


# --- BEGIN main program ---------------------------------------

# Validate command-line
options = getOptDict()
if not ('jenkinshome' in options and 'jenkinsurl' in options
        and 'job' in options and 'build' in options):
    sys.stderr.write(
        'usage: ' + sys.argv[0] + ' --jenkinshome <Jenkins home directory> ' +
        '--jenkinsurl <Jenkins base URL> --job <target job> ' +
        '--build <target build number> [--xpass <original comment about expected failure>]\n')
    sys.exit(1)

# Note: The --xpass option is for indicating that the job passed unexpectedly. Often this failure situation
# is resolved simply by no longer expecting the job to fail.

# Get information associated with the last successful target job.
last_pass_src_repo_revs = []
last_pass_build = {}
try:
     last_pass_found = getLastPassInfo(
         options['jenkinshome'], options['job'], last_pass_src_repo_revs, last_pass_build)
except Exception:
    sys.stderr.write('getLastPassInfo() failed: {}\n'.format(format_exc()))
    sys.exit(1)
except:
    sys.stderr.write('getLastPassInfo() failed (other exception): {}\n'.format(format_exc()))
    sys.exit(1)

# Get candidate committers.
cand_commits = []
for job in last_pass_src_repo_revs:
    repo_info = {}
    for repo_url in job['rev']:
        repo_info[repo_url] = {}
        repo_info[repo_url]['last_pass_rev'] = job['rev'][repo_url]['value']
        repo_info[repo_url]['commits'] = []
        job['rev'][repo_url]['vcs_ext'].addCandidateCommits(
            options['jenkinshome'], job['job'], repo_url, job['rev'][repo_url]['value'],
            repo_info[repo_url]['commits'])
    cand_commits.append( { 'job': job['job'], 'repo_info': repo_info } )

# Get fixed recipients.
fixed_recipients = getFixedRecipients(options['jenkinshome'], options['job'])

# Send email to fixed recipients and candidate committers.
report = {}
sendEmails(
    fixed_recipients, cand_commits, options['jenkinsurl'], options['job'],
    options['build'], last_pass_build, getJenkinsAdminAddr(options['jenkinshome']),
    options['xpass'] if 'xpass' in options else None, report)

# Write report to stdout.
n = len(report['cand_committers'])
sys.stdout.write('notified {} candidate committer{}:\n'.format(n, 's' if n != 1 else ''))
for x in report['cand_committers']:
    sys.stdout.write('  {}\n'.format(x))
n = len(report['fixed_recipients'])
sys.stdout.write('notified {} fixed recipient{}:\n'.format(n, 's' if n != 1 else ''))
for x in report['fixed_recipients']:
    sys.stdout.write('  {}\n'.format(x))

# --- END main program ---------------------------------------
