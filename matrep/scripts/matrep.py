#!/usr/bin/env python

import sys, os, re, urllib, json, sqlite3, calendar, time
from itertools import chain

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
                'apps': self.apps
                }, sys.stdout)

class GetApps_JSON(GetApps):
    def writeOutput(self):
        self.writeOutputAsJSON()

class GetVersions:
    def __init__(self, app):
        self.app = app

    def execute(self):
        query_result = execQuery(
            "SELECT version.name FROM app,version "
            "WHERE app.name=? AND app_id=app.id ORDER BY version.name DESC", (self.app,))
        self.versions = list(chain.from_iterable(query_result))

        self.ntests = []
        for version in self.versions:
            query_result = execQuery(
            "SELECT count(test.id) FROM app,version,test,version_test "
            "WHERE app.name=? AND version.name=? AND version.app_id=app.id AND test.app_id=app.id "
            "AND version_test.version_id=version.id AND version_test.test_id=test.id;", (self.app, version))
            self.ntests.append(query_result[0][0])

        self.ntest_results = []
        for version in self.versions:
            query_result = execQuery(
            "SELECT count(test_result.id) FROM app,version,test,version_test,test_result "
            "WHERE app.name=? AND version.name=? AND version.app_id=app.id AND test.app_id=app.id "
            "AND version_test.version_id=version.id AND version_test.test_id=test.id "
            "AND test_result.version_test_id=version_test.id;", (self.app, version))
            self.ntest_results.append(query_result[0][0])

        self.npassed = []
        for version in self.versions:
            query_result = execQuery(
            "SELECT count(test_result.id) FROM app,version,test,version_test,test_result "
            "WHERE app.name=? AND version.name=? AND version.app_id=app.id AND test.app_id=app.id "
            "AND version_test.version_id=version.id AND version_test.test_id=test.id "
            "AND test_result.version_test_id=version_test.id AND test_result.status='pass';", (self.app, version))
            self.npassed.append(query_result[0][0])

        self.nfailed = []
        for version in self.versions:
            query_result = execQuery(
            "SELECT count(test_result.id) FROM app,version,test,version_test,test_result "
            "WHERE app.name=? AND version.name=? AND version.app_id=app.id AND test.app_id=app.id "
            "AND version_test.version_id=version.id AND version_test.test_id=test.id "
            "AND test_result.version_test_id=version_test.id AND test_result.status='fail';", (self.app, version))
            self.nfailed.append(query_result[0][0])

        self.ncomments = []
        for version in self.versions:
            query_result = execQuery(
            "SELECT count(test_result.id) FROM app,version,test,version_test,test_result "
            "WHERE app.name=? AND version.name=? AND version.app_id=app.id AND test.app_id=app.id "
            "AND version_test.version_id=version.id AND version_test.test_id=test.id "
            "AND test_result.version_test_id=version_test.id AND test_result.comment!='';", (self.app, version))
            self.ncomments.append(query_result[0][0])

        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({
                'versions': self.versions,
                'ntests': self.ntests,
                'ntest_results': self.ntest_results,
                'npassed': self.npassed,
                'nfailed': self.nfailed,
                'ncomments': self.ncomments
                }, sys.stdout)

class GetVersions_JSON(GetVersions):
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

        zqr = zip(*self.query_result)
        if len(zqr) != 2:
            zqr = [[]] * 2
        self.tests = zqr[0]
        self.descrs = zqr[1]

        self.ntest_results = []
        for test in self.tests:
            query_result = execQuery(
            "SELECT count(test_result.id) FROM app,version,test,version_test,test_result "
            "WHERE app.name=? AND version.name=? AND test.name=? AND version.app_id=app.id "
            "AND test.app_id=app.id AND version_test.version_id=version.id "
            "AND version_test.test_id=test.id AND test_result.version_test_id=version_test.id;",
            (self.app, self.version, test))
            self.ntest_results.append(query_result[0][0])

        self.npassed = []
        for test in self.tests:
            query_result = execQuery(
            "SELECT count(test_result.id) FROM app,version,test,version_test,test_result "
            "WHERE app.name=? AND version.name=? AND test.name=? AND version.app_id=app.id "
            "AND test.app_id=app.id AND version_test.version_id=version.id "
            "AND version_test.test_id=test.id AND test_result.version_test_id=version_test.id "
            "AND test_result.status='pass';", (self.app, self.version, test))
            self.npassed.append(query_result[0][0])

        self.nfailed = []
        for test in self.tests:
            query_result = execQuery(
            "SELECT count(test_result.id) FROM app,version,test,version_test,test_result "
            "WHERE app.name=? AND version.name=? AND test.name=? AND version.app_id=app.id "
            "AND test.app_id=app.id AND version_test.version_id=version.id "
            "AND version_test.test_id=test.id AND test_result.version_test_id=version_test.id "
            "AND test_result.status='fail';", (self.app, self.version, test))
            self.nfailed.append(query_result[0][0])

        self.ncomments = []
        for test in self.tests:
            query_result = execQuery(
            "SELECT count(test_result.id) FROM app,version,test,version_test,test_result "
            "WHERE app.name=? AND version.name=? AND test.name=? AND version.app_id=app.id "
            "AND test.app_id=app.id AND version_test.version_id=version.id "
            "AND version_test.test_id=test.id AND test_result.version_test_id=version_test.id "
            "AND test_result.comment!='';", (self.app, self.version, test))
            self.ncomments.append(query_result[0][0])

        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({
                'tests': self.tests,
                'descrs': self.descrs,
                'ntest_results': self.ntest_results,
                'npassed': self.npassed,
                'nfailed': self.nfailed,
                'ncomments': self.ncomments
                }, sys.stdout)

class GetTests_JSON(GetTests):
    def writeOutput(self):
        self.writeOutputAsJSON()

class GetTestResults:
    def __init__(self, app, version, test):
        self.app = app
        self.version = version
        self.test = test

    def execute(self):
        self.query_result = execQuery(
            "SELECT test_result.id, test_result.timestamp, test_result.reporter, "
            "test_result.ipaddress, test_result.status, test_result.comment "
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
        if len(zqr) != 6:
            zqr = [[]] * 6
        ids = zqr[0]
        timestamps = zqr[1]
        reporters = zqr[2]
        ipaddresses = zqr[3]
        statuses = zqr[4]
        comments = zqr[5]
        json.dump({
                'ids': ids,
                'timestamps': timestamps,
                'reporters': reporters,
                'ipaddresses': ipaddresses,
                'statuses': statuses,
                'comments': comments
                }, sys.stdout)

class GetTestResults_JSON(GetTestResults):
    def writeOutput(self):
        self.writeOutputAsJSON()

class AddTestResult:
    def __init__(self, app, version, test, reporter, status, ipaddress, comment):
        self.app = app
        self.version = version
        self.test = test
        self.reporter = reporter
        self.status = status
        self.ipaddress = ipaddress
        self.comment = comment
        self.error = False

    def execute(self):

        query_result = execQuery(
            "SELECT version_test.id "
            "FROM app,version,test,version_test "
            "WHERE version.app_id=app.id "
            "AND test.app_id=app.id "
            "AND version_test.version_id=version.id "
            "AND version_test.test_id=test.id "
            "AND app.name=? AND version.name=? AND test.name=? ",
            (self.app, self.version, self.test))
        if len(query_result) == 0:
            self.error = True
        else:
            version_test_id = query_result[0][0]
            execQuery(
                "INSERT INTO test_result "
                "(version_test_id, timestamp, reporter, status, ipaddress, comment) "
                "VALUES (?, ?, ?, ?, ?, ?)",
                (version_test_id, calendar.timegm(time.gmtime()),
                 unicode(self.reporter, 'utf-8'), self.status, self.ipaddress, unicode(self.comment, 'utf-8')))
            commit()

        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({}, sys.stdout) # ### ignoring self.error for now

class AddTestResult_JSON(AddTestResult):
    def writeOutput(self):
        self.writeOutputAsJSON()

class RemoveTestResult:
    def __init__(self, id):
        self.id = id
        self.error = False

    def execute(self):
        self.versions = execQuery(
            "DELETE FROM test_result WHERE id=?", (self.id,))
        commit()
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({}, sys.stdout) # ### ignoring self.error for now

class RemoveTestResult_JSON(RemoveTestResult):
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
            '  --cmd get_test_results --app A --version V --test T | \\\n'
            '  --cmd add_test_result --app A --version V --test T --reporter R '
            '--status S --ipaddress I --comment C | \\\n'
            '  --cmd remove_test_result --id I')

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
        return GetApps_JSON()

    # --- 'get_versions' ---------------------------------
    elif cmd == 'get_versions':
        if 'app' in options:
            return GetVersions_JSON(options['app'])

    # --- 'get_tests' ---------------------------------
    elif cmd == 'get_tests':
        if ('app' in options) and ('version' in options):
            return GetTests_JSON(options['app'], options['version'])

    # --- 'get_test_results' ---------------------------------
    elif cmd == 'get_test_results':
        if ('app' in options) and ('version' in options) and ('test' in options):
            return GetTestResults_JSON(options['app'], options['version'], options['test'])

    # --- 'add_test_result' ---------------------------------
    elif cmd == 'add_test_result':
        if (('app' in options) and ('version' in options) and ('test' in options)
            and ('reporter' in options) and ('status' in options) and ('ipaddress' in options)
            and ('comment' in options)):
            return AddTestResult_JSON(
                options['app'], options['version'], options['test'],
                options['reporter'], options['status'], options['ipaddress'],
                options['comment'])

    # --- 'remove_test_result' ---------------------------------
    elif cmd == 'remove_test_result':
        if ('id' in options):
            return RemoveTestResult_JSON(options['id'])

    # No match:
    printUsageError()
    sys.exit(1)

# --- END Global functions ----------------------------------------------


# --- BEGIN Main program ----------------------------------------------

options, http_get = getOptions()
command = createCommand(options, http_get)
command.execute()

# --- END Main program ----------------------------------------------
