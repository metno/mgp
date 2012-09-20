#!/usr/bin/python

"""
This script is intended to be called from a Jenkins job whenever the job fails.
The following people are notified by email about the job failure:

  1: those who have explicitly registered to be notified about
     failures of this job, and

  2: those who have committed changes to an upstream repository
     since the last successful job build (the idea being that one of
     these changes caused the job to fail).

Assumptions:

 - Only Subversion is supported for the upstream source repositories.
   (Later the script could be extended to support other VCSs like git.
   The script should then detect the VCS automatically.)

 - A job failure can caused by a change in one of the repositories local to
   the job itself (rather than in one of the upstream ones).
   This situation must be resolved manually.
"""

import sys, re
sys.path.append('../shared/python')
from misc import getOptDict
from xml.dom.minidom import parse, parseString
from subprocess import Popen, PIPE
from datetime import datetime
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText

# --- BEGIN global functions ---------------------------------------

# For the upstream repos that could affect a target job, this function
# returns the SVN revisions associated with the last successful build
# of the target job.
#
# The return value is a list of dictionaries, one for each upstream
# job.  Each dictionary contains the upstream job name and a dictionary
# of (repo URL, revision number) combinations.
#
def getLastPassUpstreamRevs(jenkins_home, tgt_job):

    # STEP 1: For all upstream jobs, get the build number associated
    # with the latest successful build of the target job.
    fname = '{}/jobs/{}/lastSuccessful/build.xml'.format(jenkins_home, tgt_job)
    dom = parse(fname)
    upstream_job_elems = dom.getElementsByTagName('upstreamProject')
    upstream_build_elems = dom.getElementsByTagName('upstreamBuild')
    upstream_builds = []
    for i in range(len(upstream_job_elems)):
        upstream_builds.append(
            { 'job': upstream_job_elems[i].childNodes[0].nodeValue,
              'build': int(upstream_build_elems[i].childNodes[0].nodeValue) })

    # STEP 2: Get the SVN revision number for each repo
    revs = []
    for upstream_build in upstream_builds:
        fname = '{}/jobs/{}/builds/{}/build.xml'.format(
            jenkins_home, upstream_build['job'], upstream_build['build'])
        dom = parse(fname)
        repo_elems = dom.getElementsByTagName(
            'hudson.scm.SVNRevisionState')[0].getElementsByTagName('entry')
        rev = {}
        for repo_elem in repo_elems:
            rev[repo_elem.getElementsByTagName('string')[0].childNodes[0].nodeValue] = \
                int(repo_elem.getElementsByTagName('long')[0].childNodes[0].nodeValue)
        revs.append(
            { 'job': upstream_build['job'],
              'rev': rev })

    return revs


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


# For the given SVN repo ('repo_url'), this function extracts descriptions of all
# commits with revision number later than 'last_pass_rev' or later. The description records are added
# to the dictionary 'commits'.
#
def addLatestCommits(repo_url, last_pass_rev, commits):
    cmd = 'svn log {} -r{}:HEAD --xml --non-interactive --trust-server-cert'.format(
        repo_url, last_pass_rev + 1)
    p = Popen(cmd.split(), stdout = PIPE, stderr = PIPE)
    stdout, stderr = p.communicate()

    # Do nothing if the return code is non-zero (this typically means that no revisions exist
    # after last_pass_rev)
    if p.returncode != 0:
        return

    dom = parseString(stdout)
    for log_entry in dom.getElementsByTagName('logentry'):
        rev = int(log_entry.getAttribute('revision'))
        author = log_entry.getElementsByTagName('author')[0].childNodes[0].nodeValue
        date = datetime.strptime(
            log_entry.getElementsByTagName('date')[0].childNodes[0].nodeValue,
            '%Y-%m-%dT%H:%M:%S.%fZ')
        msg = log_entry.getElementsByTagName('msg')[0].childNodes[0].nodeValue
        commits.append({ 'rev': rev, 'author': author, 'date': date, 'msg': msg })


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


# Returns the URL to the description of a repository revision.
def revisionUrl(repo_url, rev):
    main_repo_name = 'diana'
    m = re.search('^https://svn.met.no/([^/]+)', repo_url)
    if m:
        main_repo_name = m.group(1)
    else:
        raise Exception('failed to extract main repo name from {}'.format(repo_url))

    url = 'https://svn.met.no/viewvc/{}?view=revision&revision={}'.format(main_repo_name, rev)
    return url


# Sends an email to the following people:
#   - fixed recipients, i.e. those who have explicitly registered to
#     be notified about build failures of the target job
#   - candidate committers, i.e. people who may potentially have
#     caused the such build failures.
#
# cand_commits is a list of dictionaries, one for each upstream job.
#     Each upstream job dictionary contains the upstream job name and
#     a dictionary of source repositories for this job. Each repo
#     dictionary contains the revision number associated with the last
#     sucessful target job and the list of dictionaries for the
#     commits made after this point.  Each commit dictionary contains
#     SVN repo URL as key and a dictionary containing the description
#     for an individual commit as value: { 'rev': ..., 'author': ...,
#     'date': ..., 'msg': ... }.
#
def sendEmails(fixed_recipients, cand_commits, jenkins_url, tgt_job, tgt_build, from_addr, report):
    # Get author names and revision numbers
    authors = set()
    for job_info in cand_commits:
        repo_info = job_info['repo_info']
        for repo_url in repo_info:
            for commit in repo_info[repo_url]['commits']:
                authors.add(commit['author'])

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

    # Create bottom part of html document
    html_bot_tmpl = ''

    for job_info in cand_commits:
        html_bot_tmpl += """
            <br/><hr><span style="font-size: 120%">Source repositories for upstream job <b>{}</b>:</span>
            <br/>
        """.format(job_info['job'])
        repo_info = job_info['repo_info']
        for repo_url in repo_info:
            html_bot_tmpl += """
                <br/>Changes in {} since the last successful build of {} (i.e. later than Rev. {}):<br/>
            """.format(repo_url, tgt_job, repo_info[repo_url]['last_pass_rev'])
            sorted_commits = sorted(repo_info[repo_url]['commits'], key=lambda item: item['rev'], reverse=True)
            if len(sorted_commits) == 0:
                html_bot_tmpl += '<b>none</b><br/>'
            else:
                html_bot_tmpl += '<table>'
                html_bot_tmpl += '<tr><th>Rev.</th><th>Author</th><th>Date</th><th>Description</th></tr>'
                for commit in sorted_commits:
                    html_bot_tmpl += (
                        '<tr class="_x_{}_x_"><td><a href={}>{}</a></td><td>{}</td>' +
                        '<td style="white-space: nowrap;">{}</td><td class=descr>{}</td></tr>').format(
                        commit['author'], revisionUrl(repo_url, commit['rev']), commit['rev'], commit['author'],
                        commit['date'].strftime('%Y-%m-%d %H:%M:%S'), commit['msg'])
                html_bot_tmpl += '</table>'

    html_bot_tmpl += '</body>'
    html_bot_tmpl += '</html>'

    tgt_url = '{}/job/{}/{}/console'.format(jenkins_url, tgt_job, tgt_build)
    report['cand_committers'] = []
    report['fixed_recipients'] = []

    # Send a separate email to each author
    for author in authors:
        p = re.compile('_x_{}_x_'.format(author))
        html_bot = p.sub('hilight', html_bot_tmpl) # highlight commits made by this author
        html_mid = """
            You receive this email because you may potentially have caused
            the failure of <a href="{}"><b>{}</b> #{}</a>.
            <br/><br/>
            Please investigate.
            <br/>
        """.format(tgt_url, tgt_job, tgt_build)
        html = html_top + html_mid + html_bot
        to_addr = '{}@met.no'.format(author) # assuming the domain 'met.no' works for now
        sendEmail(from_addr, to_addr,
                  'Jenkins alert: {} #{} failed - please investigate'.format(tgt_job, tgt_build), html)
        report['cand_committers'].append(to_addr)

    # Send a separate email to each fixed recipient
    for recp in fixed_recipients:
        html_mid = """
            You receive this email because you have registered to be notified whenever
            <b>{}</b> fails (in this case <a href="{}">Build #{}</a>).
            <br/><br/>
            Candidate committers (listed below) have been notified separately.
            <br/>
        """.format(tgt_job, tgt_url, tgt_build)
        html = html_top + html_mid + html_bot_tmpl
        sendEmail(from_addr, recp, 'Jenkins alert: {} #{} failed'.format(tgt_job, tgt_build), html)
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
        '--build <target build number>\n')
    sys.exit(1)


# Step 1: Get the upstream SVN revisions associated with the last successful target job.
try:
    last_pass_upstream_revs = getLastPassUpstreamRevs(
        options['jenkinshome'], options['job'])
except Exception as e:
    sys.stderr.write('lastPassUpstreamRevs() failed: {}\n'.format(e))
    sys.exit(1)
except:
    sys.stderr.write('lastPassUpstreamRevs() failed (other exception): {}\n'.format(sys.exc_info()))
    sys.exit(1)


# Step 2: Get candidate committers.
cand_commits = []
for upstream_job in last_pass_upstream_revs:
    repo_info = {}
    for repo_url in upstream_job['rev']:
        repo_info[repo_url] = {}
        repo_info[repo_url]['last_pass_rev'] = upstream_job['rev'][repo_url]
        repo_info[repo_url]['commits'] = []
        addLatestCommits(repo_url, upstream_job['rev'][repo_url], repo_info[repo_url]['commits'])
    cand_commits.append( { 'job': upstream_job['job'], 'repo_info': repo_info } )

# Step 3: Get fixed recipients.
fixed_recipients = getFixedRecipients(options['jenkinshome'], options['job'])


# Step 4: Send email to fixed recipients and candidate committers.
report = {}
sendEmails(
    fixed_recipients, cand_commits, options['jenkinsurl'], options['job'],
    options['build'], getJenkinsAdminAddr(options['jenkinshome']), report)


# Step 5: Write report to stdout.
n = len(report['cand_committers'])
sys.stdout.write('notified {} candidate committer{}:\n'.format(n, 's' if n != 1 else ''))
for x in report['cand_committers']:
    sys.stdout.write('  {}\n'.format(x))
n = len(report['fixed_recipients'])
sys.stdout.write('notified {} fixed recipient{}:\n'.format(n, 's' if n != 1 else ''))
for x in report['fixed_recipients']:
    sys.stdout.write('  {}\n'.format(x))

# --- END main program ---------------------------------------
