#!/usr/bin/python

"""
Given two Jenkins jobs, one product build job and one test job to test
the product, this script is intended to be called by the main script
of the test job whenever the test fails.

The script notifies a group of people with an email whenever the test
fails. Recipients are of two types:

  1: those who have explicitly registered to be notified about test
     failures, and

  2: those who have committed changes to the product since the last
     successful test run (the idea being that one of these changes
     caused the test to fail).

Assumptions:

 - Only Subversion is supported for the product source repositories.
   (Later the script could be extended to support other VCSs like git.
   The script should then detect the VCS automatically.)

 - A test failure can caused by a change in the test itself (rather
   than a change in the product) must be resolved manually.
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

# For the repos used to build the given product, this function returns the SVN revisions associated with
# the last successful run of the given test. The return value is a dictionary with repo URLs as keys
# and revision numbers as values.
def getLastPassProdRevs(jenkins_home, product_job, test_job):

    # STEP 1: Get the product job build number associated with the
    # latest successful test run (note: we assume that the nearest
    # upstream job is the first one to appear in the XML file)
    fname = '{}/jobs/{}/lastSuccessful/build.xml'.format(jenkins_home, test_job)
    dom = parse(fname)
    # ... verify that the product job name matches
    upstream_job_elem = dom.getElementsByTagName('upstreamProject')[0]
    upstream_job_name = upstream_job_elem.childNodes[0].nodeValue
    if upstream_job_name != product_job:
        raise Exception('nearest upstream job (\'{}\') does not match \'{}\''.format(
                upstream_job_name, product_job))
    # ... get the build number
    upstream_build_elem = dom.getElementsByTagName('upstreamBuild')[0]
    upstream_build_no = int(upstream_build_elem.childNodes[0].nodeValue)

    # STEP 2: Get the SVN revision number for each repo URL
    rev =  {}
    fname = '{}/jobs/{}/builds/{}/build.xml'.format(
        jenkins_home, product_job, upstream_build_no)
    dom = parse(fname)
    repo_elems = dom.getElementsByTagName(
        'hudson.scm.SVNRevisionState')[0].getElementsByTagName('entry')
    for repo_elem in repo_elems:
        rev[repo_elem.getElementsByTagName('string')[0].childNodes[0].nodeValue] = \
            int(repo_elem.getElementsByTagName('long')[0].childNodes[0].nodeValue)

    return rev


# Returns the list of email addresses of the people registered to be notified whenever a test job
# fails.
def getFixedRecipients(jenkins_home, test_job):
    fname = '{}/jobs/{}/config.xml'.format(jenkins_home, test_job)
    dom = parse(fname)
    recp_elem = dom.getElementsByTagName('recipients')[0]
    recipients = recp_elem.childNodes[0].nodeValue.split()
    return recipients


# For the given SVN repo ('repo_url'), this function extracts descriptions of all
# commits with revision number 'lo_rev' or later. The description records are added
# to the dictionary 'commits'.
#
def addLatestCommits(repo_url, lo_rev, commits):
    # FINAL VERSION:
    #    cmd = 'svn log {} -r{}:HEAD --xml'.format(repo_url, lo_rev)
    # ### HACK USED FOR TESTING:
    sys.stderr.write('(--- hack enabled - remove before pushing ---)\n')
    cmd = 'svn log {} -r{}:HEAD --xml'.format(repo_url, lo_rev - 4)

    p = Popen(cmd.split(), stdout = PIPE, stderr = PIPE)
    stdout, stderr = p.communicate()

    # Do nothing if the return code is non-zero (this typically means that lo_rev is beyond
    # the available range)
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
    if to_addr != 'joa@met.no' and to_addr != 'jo.asplin@met.no':
        print '### sendEmail() SKIPPED ... to_addr:', to_addr
        return

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
    msg.attach(part1)
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
#   - fixed recipients, i.e. those who have explicitly registered to be notified about test failures
#   - candidate committers, i.e. people who may potentially have caused the test failure.
# cand_commits is a dictionary with the SVN repo URL as key and a dictionary containing the
# description for an individual commit as value: { 'rev': ..., 'author': ..., 'date': ..., 'msg': ... }
def sendEmails(fixed_recipients, cand_commits, test_name, test_url, report):
    # Get author names and revision numbers
    authors = set()
    revs = set()
    for repo_url in cand_commits:
        for commit in cand_commits[repo_url]:
            authors.add(commit['author'])
            revs.add(commit['rev'])
    authors_maxlen = len(max(authors, key=lambda item: len(item)))
    revs_maxlen = len(str(max(revs, key=lambda item: len(str(item)))))

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
    for repo_url in cand_commits:
        html_bot_tmpl += '<br/>Changes in {} since the last successful test run:<br/>'.format(repo_url)
        html_bot_tmpl += '<table>'
        html_bot_tmpl += '<tr><th>Rev</th><th>Author</th><th>Date</th><th>Description</th></tr>'
        sorted_commits = sorted(cand_commits[repo_url], key=lambda item: item['rev'], reverse=True)
        for commit in sorted_commits:
            html_bot_tmpl += ('<tr class="_x_{}_x_"><td><a href={}>{}</a></td><td>{}</td>' +
                          '<td style="white-space: nowrap;">{}</td><td class=descr>{}</td></tr>').format(
                commit['author'], revisionUrl(repo_url, commit['rev']), commit['rev'], commit['author'],
                commit['date'].strftime('%Y-%m-%d %H:%M:%S'), commit['msg'])
        html_bot_tmpl += '</table>'
    html_bot_tmpl += '</body>'
    html_bot_tmpl += '</html>'

    from_addr = 'Jenkins daemon (WMS project)<joa@met.no>'
    report['cand_committers'] = []
    report['fixed_recipients'] = []

    # Send a separate email to each author
    for author in authors:
        p = re.compile('_x_{}_x_'.format(author))
        html_bot = p.sub('hilight', html_bot_tmpl) # highlight commits made by this author
        html_mid = """
            You receive this email because you may potentially have caused
            <a href="{}">this test failure</a>.
            <br/><br/>
            Please investigate.
            <br/>
        """.format(test_url)
        html = html_top + html_mid + html_bot
        to_addr = '{}@met.no'.format(author)
        sendEmail(from_addr, to_addr,
                  'Jenkins alert: {} failed - please investigate'.format(test_name), html)
        report['cand_committers'].append(to_addr)

    # Send a separate email to each fixed recipient
    for recp in fixed_recipients:
        html_mid = """
            You receive this email because you have registered to be notified whenever
            {} <a href="{}">fails</a>.
            <br/><br/>
            Candidate committers (listed below) have been notified separately.
            <br/>
        """.format(test_name, test_url)
        html = html_top + html_mid + html_bot_tmpl
        sendEmail(from_addr, recp, 'Jenkins alert: {} failed'.format(test_name), html)
        report['fixed_recipients'].append(recp)

# --- END global functions ---------------------------------------


# --- BEGIN main program ---------------------------------------

# Validate command-line
options = getOptDict()
if not ('jenkinshome' in options and 'jenkinsurl' in options and 'product' in options
        and 'test' in options and 'testbuild' in options):
    sys.stderr.write(
        'usage: ' + sys.argv[0] + ' --jenkinshome <Jenkins home directory> ' +
        '--jenkinsurl <Jenkins base URL> --product <name of product build job> ' +
        '--test <name of test job> --testbuild <current test build number>\n')
    sys.exit(1)


# Step 1: Get the product SVN revisions associated with the last successful test run.
try:
    lastPassProdRevs = getLastPassProdRevs(options['jenkinshome'], options['product'], options['test'])
except Exception as e:
    sys.stderr.write('lastPassProdRevs() failed: {}\n'.format(e))
    sys.exit(1)
except:
    sys.stderr.write('lastPassProdRevs() failed (other exception): {}\n'.format(sys.exc_info()))
    sys.exit(1)


# Step 2: Get candidate committers.
cand_commits = {}
for repo_url in lastPassProdRevs:
    cand_commits[repo_url] = []
    addLatestCommits(repo_url, lastPassProdRevs[repo_url] + 1, cand_commits[repo_url])


# Step 3: Get fixed recipients.
fixed_recipients = getFixedRecipients(options['jenkinshome'], options['test'])


# Step 4: Send email to fixed recipients and candidate committers.
report = {}
print cand_commits
sys.exit(1)
sendEmails(fixed_recipients, cand_commits, options['test'], '{}/job/{}/{}/console'.format(
        options['jenkinsurl'], options['test'], options['testbuild']), report)


# Step 5: Write report to stdout.
sys.stdout.write('notified {} candidate committers:\n'.format(len(report['cand_committers'])))
for x in report['cand_committers']:
    sys.stdout.write('  {}\n'.format(x))
sys.stdout.write('notified {} fixed recipients:\n'.format(len(report['fixed_recipients'])))
for x in report['fixed_recipients']:
    sys.stdout.write('  {}\n'.format(x))

# --- END main program ---------------------------------------
