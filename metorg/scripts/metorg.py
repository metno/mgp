#!/usr/bin/env python

import sys, os, re, urllib, json, sqlite3, calendar, time
from subprocess import Popen, PIPE

# --- BEGIN Global classes ----------------------------------------------

class Command:
    def writeOutput(self):
        self.writeOutputAsJSON() if self.http_get else self.writeOutputAsPlainText()


class GetUsers(Command):
    def __init__(self, http_get):
        self.http_get = http_get
        self.error = None

    def execute(self):
        self.users, self.error = getLDAPUsers(['met-vnn', 'met-vv', 'met-va'])
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({
                'users': self.users,
                'error': unicode(self.error, 'utf-8') if self.error else None
                }, sys.stdout)

    def writeOutputAsPlainText(self):
        if self.error:
            print 'error: ' + self.error
        else: 
            for user in self.users:
                print user

# --- END Global classes ----------------------------------------------


# --- BEGIN Global functions ----------------------------------------------

# Returns the sorted union of memberUid values from a list of LDAP groups.
def getLDAPUsers(groups):
    users = []
    for group in groups:
        p1 = Popen(
            ['ldapsearch', '-x', '-H', 'ldaps://ldap-dev.met.no', '-b',
             'ou=internal,dc=met,dc=no', 'cn={}'.format(group)], stdout=PIPE, stderr=PIPE)
        p2 = Popen(['grep', 'memberUid'], stdin=p1.stdout, stdout=PIPE, stderr=PIPE)
        p3 = Popen(['cut', '-f2', '-d '], stdin=p2.stdout, stdout=PIPE, stderr=PIPE)
        stdout, stderr = p3.communicate()
        if p3.returncode != 0:
            return [], 'ldapsearch failed with exit code {}.\nSTDOUT: \'{}\'\nSTDERR: \'{}\''.format(
                p3.returncode, stdout, stderr)
        users = list(set(users) | set(stdout.split()))
    return sorted(users), None

# Connects to the database and creates a global cursor.
def connectDatabase():
    fname = '/var/www/metorg_home/metorg.db' # for now
    try:
        conn = sqlite3.connect(fname)
        conn.execute('pragma foreign_keys=ON')
    except:
        print "failed to connect to the database:", sys.exc_info()
        sys.exit(1)

    global cursor
    cursor = conn.cursor()

# Commits everything that has been written to the database.
def commit():
    if not "cursor" in globals():
        return
    cursor.connection.commit()

# Executes a query against the database. args contains the arguments to be
# passed to the query. Returns any result set iff fetch_results is true.
def execQuery(query, args, fetch_results = True):
    if not 'cursor' in globals():
        connectDatabase()
    assert 'cursor' in globals()

    try:
        cursor.execute(query, args)
        if fetch_results:
            return cursor.fetchall()
    except sqlite3.Error:
        print 'query failed: >' + query + '<'
        print 'reason:', str(sys.exc_info())
        sys.exit(1)

def printJSONHeader():
    print 'Content-type: text/json\n'

def printErrorAsJSON(error):
    printJSONHeader()
    print '{\"error\": \"' + error + '\"}\n'

# Returns a 2-tuple consisting of:
# 1: an option dictionary, and
# 2: a flag that is true iff the QUERY_STRING environment variable is
#    present (i.e. that the script is invoked as a CGI-script for a
#    HTTP GET request).
#
# The option dictionary is extracted from either the QUERY_STRING environment
# variable (first priority) or command-line arguments (second priority).
# In the latter case, the options must be of the form
# ... --<opt1> <val1> ... --<optN> <valN> ...
def getOptions():

    def getOptDictFromQueryString(qs):
        options = {}
        for sq in qs.split('&'):
            keyval = sq.split('=')
            options[keyval[0]] = urllib.unquote(keyval[1])
        return options

    def getOptDictFromCommandLine():
        options = {}
        p = re.compile('^--(.+)$')
        key = None
        for arg in sys.argv[1:]:
            if key != None:
                options[key] = arg
            m = p.match(arg)
            if m:
                key = m.group(1)
                if key in ['help', 'force']: # value-less options
                    options[key] = 1
                    key = None
            else:
                key = None
        return options

    qs = 'QUERY_STRING'
    if qs in os.environ:
        return (getOptDictFromQueryString(os.environ[qs]), True)
    else:
        return (getOptDictFromCommandLine(), False)

# Returns a command instance.
def createCommand(options, http_get):

    def printUsageError():
        error = (
            'usage: ' + sys.argv[0] + '\\\n'
            '  --cmd get_users')

        if http_get:
            printErrorAsJSON('usage error')
        else:
            print error

    # Check for mandatory 'cmd' argument:
    if 'cmd' in options:
        cmd = options['cmd']
    else:
        printUsageError()
        sys.exit(1)

    # return the command if possible:

    # --- 'get_users' ---------------------------------
    if cmd == 'get_users':
        return GetUsers(http_get)

    # No match:
    printUsageError()
    sys.exit(1)

# --- END Global functions ----------------------------------------------


# --- BEGIN Main program ----------------------------------------------

options, http_get = getOptions()
command = createCommand(options, http_get)
command.execute()

# --- END Main program ----------------------------------------------
