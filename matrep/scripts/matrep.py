#!/usr/bin/env python

import sys, os, re, urllib, json, sqlite3

# --- BEGIN Global classes ----------------------------------------------

class GetApps:
    def __init__(self):
        pass

    def execute(self):
        self.apps = execQuery('SELECT name FROM app ORDER by name', ())
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({
                'apps': self.apps,
                }, sys.stdout)

class GetAppsAsJSON(GetApps):
    def writeOutput(self):
        self.writeOutputAsJSON()

class GetVersions:
    def __init__(self, app):
        self.app = app

    def execute(self):
        self.versions = execQuery(
            "SELECT version.name FROM app,version "
            "WHERE app_id=app.id AND app.name=? ORDER BY version.name DESC", (self.app,))
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({
                'versions': self.versions,
                }, sys.stdout)

class GetVersionsAsJSON(GetVersions):
    def writeOutput(self):
        self.writeOutputAsJSON()

class GetTests:
    def __init__(self, app, version):
        self.app = app
        self.version = version

    def execute(self):
        self.query_result = execQuery(
            "SELECT test.name, test.description FROM app,version,test,version_test "
            "WHERE version.app_id=app.id "
            "AND test.app_id=app.id "
            "AND version_test.version_id=version.id "
            "AND version_test.test_id=test.id "
            "AND app.name=? AND version.name=? "
            "ORDER BY test.name", (self.app, self.version))
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        zqr = zip(*self.query_result)
        if (len(zqr) != 2):
            zqr = [[]] * 2
        tests = zqr[0]
        descrs = zqr[1]
        json.dump({
                'tests': tests,
                'descrs': descrs,
                }, sys.stdout)

class GetTestsAsJSON(GetTests):
    def writeOutput(self):
        self.writeOutputAsJSON()

class GetTestResults:
    def __init__(self, app, version, test):
        self.app = app
        self.version = version
        self.test = test

    def execute(self):
        self.query_result = execQuery(
            "SELECT test_result.timestamp, test_result.reporter, "
            "test_result.ipaddress, test_result.status, test_result.description "
            "FROM app,version,test,version_test,test_result "
            "WHERE version.app_id=app.id "
            "AND test.app_id=app.id "
            "AND version_test.version_id=version.id "
            "AND version_test.test_id=test.id "
            "AND version_test_id=version_test.id "
            "AND app.name=? AND version.name=? AND test.name=? "
            "ORDER BY test_result.timestamp", (self.app, self.version, self.test))
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        zqr = zip(*self.query_result)
        if (len(zqr) != 5):
            zqr = [[]] * 5
        timestamps = zqr[0]
        reporters = zqr[1]
        ipaddresses = zqr[2]
        statuses = zqr[3]
        descrs = zqr[4]
        json.dump({
                'timestamps': timestamps,
                'reporters': reporters,
                'ipaddresses': ipaddresses,
                'statuses': statuses,
                'descrs': descrs,
                }, sys.stdout)

class GetTestResultsAsJSON(GetTestResults):
    def writeOutput(self):
        self.writeOutputAsJSON()

# --- END Global classes ----------------------------------------------


# --- BEGIN Global functions ----------------------------------------------

# Connects to the database and creates a global cursor.
def connectDatabase():
    fname = '/var/www/matrep_home/matrep.db' # for now
    try:
        conn = sqlite3.connect(fname)
    except:
        print "failed to connect to the database:", sys.exc_info()
        sys.exit(1)

    global cursor
    cursor = conn.cursor()

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
                # support "--help" as the only value-less option:
                if key == 'help':
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
            '  --cmd get_apps | \\\n'
            '  --cmd get_versions --app A | \\\n'
            '  --cmd get_tests --app A --version V | \\\n'
            '  --cmd get_test_results --app A --version V --test T')

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

    # --- 'get_apps' ---------------------------------
    if cmd == 'get_apps':
        return GetAppsAsJSON()

    # --- 'get_versions' ---------------------------------
    elif cmd == 'get_versions':
        if 'app' in options:
            return GetVersionsAsJSON(options['app'])

    # --- 'get_tests' ---------------------------------
    elif cmd == 'get_tests':
        if ('app' in options) and ('version' in options):
            return GetTestsAsJSON(options['app'], options['version'])

    # --- 'get_test_results' ---------------------------------
    elif cmd == 'get_test_results':
        if ('app' in options) and ('version' in options) and ('test' in options):
            return GetTestResultsAsJSON(options['app'], options['version'], options['test'])

    # No match:
    printUsageError()
    sys.exit(1)

# --- END Global functions ----------------------------------------------


# --- BEGIN Main program ----------------------------------------------

options, http_get = getOptions()
command = createCommand(options, http_get)
command.execute()

# --- END Main program ----------------------------------------------
