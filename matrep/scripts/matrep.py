#!/usr/bin/env python

import sys, os, re, urllib, json, sqlite3, calendar, time
from itertools import chain

# --- BEGIN Global classes ----------------------------------------------

class Command:
    def writeOutput(self):
        self.writeOutputAsJSON() if self.http_get else self.writeOutputAsPlainText()


class GetApps(Command):
    def __init__(self, http_get):
        self.http_get = http_get

    def execute(self):
        query_result = execQuery('SELECT name FROM app ORDER by name', ())
        self.apps = list(chain.from_iterable(query_result))
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({
                'apps': self.apps
                }, sys.stdout)

    def writeOutputAsPlainText(self):
        for app in self.apps:
            print app


class AddApp(Command):
    def __init__(self, http_get, app):
        self.http_get = http_get
        self.app = app
        self.error = None

    def doExecute(self):
        self.app = self.app.strip()
        if self.app == '':
            self.error = 'empty app name'
            return

        app_id = getAppID(self.app)
        if app_id >= 0:
            self.error = 'app ' + self.app + ' already exists'
            return

        execQuery(
            "INSERT INTO app "
            "(name) "
            "VALUES (?)",
            (unicode(self.app, 'utf-8'),))
        commit()

    def execute(self):
        self.doExecute()
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({}, sys.stdout) # ### ignoring self.error for now

    def writeOutputAsPlainText(self):
        if self.error != None:
            print 'error: ' + self.error


class RemoveApp(Command):
    def __init__(self, http_get, app):
        self.http_get = http_get
        self.app = app
        self.error = None

    def doExecute(self):
        app_id = getAppID(self.app)
        if app_id < 0:
            self.error = 'app ' + self.app + ' not found'
            return

        versions = getVersions(app_id)
        if len(versions) > 0:
            self.error = (
                'the app ' + self.app + ' is already associated with the following version(s): ' +
                ", ".join(str(v) for v in versions))
            return

        tests = getTests(app_id)
        if len(tests) > 0:
            self.error = (
                'the app ' + self.app + ' is already associated with the following test(s): ' +
                ", ".join(str(t) for t in tests))
            return

        execQuery("DELETE FROM app WHERE id=?;", (app_id,))
        commit()

    def execute(self):
        self.doExecute()
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({}, sys.stdout) # ### ignoring self.error for now

    def writeOutputAsPlainText(self):
        if self.error != None:
            print 'error: ' + self.error


class RenameApp(Command):
    def __init__(self, http_get, old, new):
        self.http_get = http_get
        self.old = old
        self.new = new
        self.error = None

    def doExecute(self):
        self.new = self.new.strip()
        if self.new == '':
            self.error = 'empty app name'
            return

        old_id = getAppID(self.old)
        if old_id < 0:
            self.error = 'app not found: ' + self.old
            return

        new_id = getAppID(self.new)
        if new_id >= 0:
            self.error = 'app ' + self.new + ' already exists'
            return

        execQuery(
            "UPDATE app SET name=? WHERE id=?;",
            (unicode(self.new, 'utf-8'), old_id))
        commit()

    def execute(self):
        self.doExecute()
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({}, sys.stdout) # ### ignoring self.error for now

    def writeOutputAsPlainText(self):
        if self.error != None:
            print 'error: ' + self.error


class GetVersions(Command):
    def __init__(self, http_get, app):
        self.http_get = http_get
        self.app = app
        self.error = None

    def doExecute(self):
        app_id = getAppID(self.app)
        if app_id < 0:
            self.error = 'app ' + self.app + ' not found'
            return

        query_result = execQuery(
            "SELECT version.name FROM app,version "
            "WHERE app.id=? AND app_id=app.id ORDER BY version.name DESC", (app_id,))
        self.versions = list(chain.from_iterable(query_result))

        self.ntests = []
        for version in self.versions:
            query_result = execQuery(
            "SELECT count(test.id) FROM app,version,test,version_test "
            "WHERE app.id=? AND version.name=? AND version.app_id=app.id AND test.app_id=app.id "
            "AND version_test.version_id=version.id AND version_test.test_id=test.id;", (app_id, version))
            self.ntests.append(query_result[0][0])

        self.ntest_results = []
        for version in self.versions:
            query_result = execQuery(
            "SELECT count(test_result.id) FROM app,version,test,version_test,test_result "
            "WHERE app.id=? AND version.name=? AND version.app_id=app.id AND test.app_id=app.id "
            "AND version_test.version_id=version.id AND version_test.test_id=test.id "
            "AND test_result.version_test_id=version_test.id;", (app_id, version))
            self.ntest_results.append(query_result[0][0])

        self.npassed = []
        for version in self.versions:
            query_result = execQuery(
            "SELECT count(test_result.id) FROM app,version,test,version_test,test_result "
            "WHERE app.id=? AND version.name=? AND version.app_id=app.id AND test.app_id=app.id "
            "AND version_test.version_id=version.id AND version_test.test_id=test.id "
            "AND test_result.version_test_id=version_test.id AND test_result.status='pass';", (app_id, version))
            self.npassed.append(query_result[0][0])

        self.nfailed = []
        for version in self.versions:
            query_result = execQuery(
            "SELECT count(test_result.id) FROM app,version,test,version_test,test_result "
            "WHERE app.id=? AND version.name=? AND version.app_id=app.id AND test.app_id=app.id "
            "AND version_test.version_id=version.id AND version_test.test_id=test.id "
            "AND test_result.version_test_id=version_test.id AND test_result.status='fail';", (app_id, version))
            self.nfailed.append(query_result[0][0])

        self.ncomments = []
        for version in self.versions:
            query_result = execQuery(
            "SELECT count(test_result.id) FROM app,version,test,version_test,test_result "
            "WHERE app.id=? AND version.name=? AND version.app_id=app.id AND test.app_id=app.id "
            "AND version_test.version_id=version.id AND version_test.test_id=test.id "
            "AND test_result.version_test_id=version_test.id AND test_result.comment!='';", (app_id, version))
            self.ncomments.append(query_result[0][0])

    def execute(self):
        self.doExecute()
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

    def writeOutputAsPlainText(self):
        if self.error != None:
            print 'error: ' + self.error
        else:
            for version in self.versions:
                print version


class AddVersion(Command):
    def __init__(self, http_get, app, version):
        self.http_get = http_get
        self.app = app
        self.version = version
        self.error = None

    def doExecute(self):
        self.version = self.version.strip()
        if self.version == '':
            self.error = 'empty version name'
            return

        app_id = getAppID(self.app)
        if app_id < 0:
            self.error = 'app ' + self.app + ' not found'
            return

        version_id = getVersionID(app_id, self.version)
        if version_id >= 0:
            self.error = 'app ' + self.app + ' already contains version ' + self.version
            return

        execQuery(
            "INSERT INTO version "
            "(app_id, name) "
            "VALUES (?, ?)",
            (app_id, unicode(self.version, 'utf-8')))
        commit()

    def execute(self):
        self.doExecute()
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({}, sys.stdout) # ### ignoring self.error for now

    def writeOutputAsPlainText(self):
        if self.error != None:
            print 'error: ' + self.error


class RemoveVersion(Command):
    def __init__(self, http_get, app, version):
        self.http_get = http_get
        self.app = app
        self.version = version
        self.error = None

    def doExecute(self):
        app_id = getAppID(self.app)
        if app_id < 0:
            self.error = 'app ' + self.app + ' not found'
            return

        version_id = getVersionID(app_id, self.version)
        if version_id < 0:
            self.error = 'app ' + self.app + ' contains no such version: ' + self.version
            return

        query_result = execQuery(
            "SELECT test.name FROM app,version,test,version_test "
            "WHERE version.app_id=app.id AND test.app_id=app.id "
            "AND version_test.version_id=version.id AND version_test.test_id=test.id "
            "AND app.id=? AND version.id=?;", (app_id, version_id))
        if len(query_result) > 0:
            self.error = (
                'the version ' + self.version + ' is already associated with the following test(s): ' +
                ", ".join(str(t) for t in zip(*query_result)[0]))
            return

        execQuery("DELETE FROM version WHERE app_id=? AND id=?;", (app_id, version_id))
        commit()

    def execute(self):
        self.doExecute()
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({}, sys.stdout) # ### ignoring self.error for now

    def writeOutputAsPlainText(self):
        if self.error != None:
            print 'error: ' + self.error


class RenameVersion(Command):
    def __init__(self, http_get, app, old, new):
        self.http_get = http_get
        self.app = app
        self.old = old
        self.new = new
        self.error = None

    def doExecute(self):
        self.new = self.new.strip()
        if self.new == '':
            self.error = 'empty version name'
            return

        app_id = getAppID(self.app)
        if app_id < 0:
            self.error = 'app ' + self.app + ' not found'
            return

        old_id = getVersionID(app_id, self.old)
        if old_id < 0:
            self.error = 'app ' + self.app + ' contains no such version: ' + self.old
            return

        new_id = getVersionID(app_id, self.new)
        if new_id >= 0:
            self.error = 'app ' + self.app + ' already contains version: ' + self.new
            return

        execQuery(
            "UPDATE version SET name=? WHERE app_id=? AND id=?;",
            (unicode(self.new, 'utf-8'), app_id, old_id))
        commit()

    def execute(self):
        self.doExecute()
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({}, sys.stdout) # ### ignoring self.error for now

    def writeOutputAsPlainText(self):
        if self.error != None:
            print 'error: ' + self.error


class GetTests(Command):
    def __init__(self, http_get, app, version):
        self.http_get = http_get
        self.app = app
        self.version = version
        self.error = None

    def doExecute(self):
        app_id = getAppID(self.app)
        if app_id < 0:
            self.error = 'app ' + self.app + ' not found'
            return

        version_id = -1
        if self.version != None:
            version_id = getVersionID(app_id, self.version)
            if version_id < 0:
                self.error = 'app ' + self.app + ' contains no such version: ' + self.version
                return

        if version_id >= 0:
            self.query_result = execQuery(
                "SELECT test.name, test.description FROM app,version,test,version_test "
                "WHERE version.app_id=app.id "
                "AND test.app_id=app.id "
                "AND version_test.version_id=version.id "
                "AND version_test.test_id=test.id "
                "AND app.id=? AND version.id=? "
                "ORDER BY test.name", (app_id, version_id))
        else:
            self.query_result = execQuery(
                "SELECT test.name, test.description FROM app,test "
                "WHERE test.app_id=app.id "
                "AND app.id=? "
                "ORDER BY test.name", (app_id,))

        zqr = zip(*self.query_result)
        if len(zqr) != 2:
            zqr = [[]] * 2
        self.tests = zqr[0]
        self.descrs = zqr[1]

        self.ntest_results = []
        for test in self.tests:
            if version_id >= 0:
                query_result = execQuery(
                    "SELECT count(test_result.id) FROM app,version,test,version_test,test_result "
                    "WHERE app.id=? AND version.id=? AND test.name=? AND version.app_id=app.id "
                    "AND test.app_id=app.id AND version_test.version_id=version.id "
                    "AND version_test.test_id=test.id AND test_result.version_test_id=version_test.id;",
                    (app_id, version_id, test))
            else:
                query_result = execQuery(
                    "SELECT count(test_result.id) FROM app,test,version_test,test_result "
                    "WHERE app.id=? AND test.name=? "
                    "AND test.app_id=app.id AND version_test.test_id=test.id "
                    "AND test_result.version_test_id=version_test.id;",
                    (app_id, test))
            self.ntest_results.append(query_result[0][0])

        self.npassed = []
        for test in self.tests:
            if version_id >= 0:
                query_result = execQuery(
                    "SELECT count(test_result.id) FROM app,version,test,version_test,test_result "
                    "WHERE app.id=? AND version.id=? AND test.name=? AND version.app_id=app.id "
                    "AND test.app_id=app.id AND version_test.version_id=version.id "
                    "AND version_test.test_id=test.id AND test_result.version_test_id=version_test.id "
                    "AND test_result.status='pass';", (app_id, version_id, test))
            else:
                query_result = execQuery(
                    "SELECT count(test_result.id) FROM app,test,version_test,test_result "
                    "WHERE app.id=? AND test.name=? "
                    "AND test.app_id=app.id AND version_test.test_id=test.id "
                    "AND test_result.version_test_id=version_test.id "
                    "AND test_result.status='pass';", (app_id, test))
            self.npassed.append(query_result[0][0])

        self.nfailed = []
        for test in self.tests:
            if version_id >= 0:
                query_result = execQuery(
                    "SELECT count(test_result.id) FROM app,version,test,version_test,test_result "
                    "WHERE app.id=? AND version.id=? AND test.name=? AND version.app_id=app.id "
                    "AND test.app_id=app.id AND version_test.version_id=version.id "
                    "AND version_test.test_id=test.id AND test_result.version_test_id=version_test.id "
                    "AND test_result.status='fail';", (app_id, version_id, test))
            else:
                query_result = execQuery(
                    "SELECT count(test_result.id) FROM app,test,version_test,test_result "
                    "WHERE app.id=? AND test.name=? "
                    "AND test.app_id=app.id AND version_test.test_id=test.id "
                    "AND test_result.version_test_id=version_test.id "
                    "AND test_result.status='fail';", (app_id, test))
            self.nfailed.append(query_result[0][0])

        self.ncomments = []
        for test in self.tests:
            if version_id >= 0:
                query_result = execQuery(
                    "SELECT count(test_result.id) FROM app,version,test,version_test,test_result "
                    "WHERE app.id=? AND version.id=? AND test.name=? AND version.app_id=app.id "
                    "AND test.app_id=app.id AND version_test.version_id=version.id "
                    "AND version_test.test_id=test.id AND test_result.version_test_id=version_test.id "
                    "AND test_result.comment!='';", (app_id, version_id, test))
            else:
                query_result = execQuery(
                    "SELECT count(test_result.id) FROM app,test,version_test,test_result "
                    "WHERE app.id=? AND test.name=? "
                    "AND test.app_id=app.id AND version_test.test_id=test.id "
                    "AND test_result.version_test_id=version_test.id "
                    "AND test_result.comment!='';", (app_id, test))
            self.ncomments.append(query_result[0][0])

    def execute(self):
        self.doExecute()
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

    def writeOutputAsPlainText(self):
        if self.error != None:
            print 'error: ' + self.error
        else:
            for test in self.tests:
                print test


class AddTest(Command):
    def __init__(self, http_get, app, test, descr):
        self.http_get = http_get
        self.app = app
        self.test = test
        self.descr = descr
        self.error = None

    def doExecute(self):
        self.test = self.test.strip()
        if self.test == '':
            self.error = 'empty test name'
            return

        app_id = getAppID(self.app)
        if app_id < 0:
            self.error = 'app ' + self.app + ' not found'
            return

        test_id = getTestID(app_id, self.test)
        if test_id >= 0:
            self.error = 'app ' + self.app + ' already contains test ' + self.test
            return

        descr_content = readTestDescription(self.descr)
        if (descr_content == ''):
            self.error = 'empty description'
            return

        execQuery(
            "INSERT INTO test "
            "(app_id, name, description) "
            "VALUES (?, ?, ?)",
            (app_id, unicode(self.test, 'utf-8'), unicode(descr_content, 'utf-8')))
        commit()

    def execute(self):
        self.doExecute()
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({}, sys.stdout) # ### ignoring self.error for now

    def writeOutputAsPlainText(self):
        if self.error != None:
            print 'error: ' + self.error


class RemoveTest(Command):
    def __init__(self, http_get, app, test):
        self.http_get = http_get
        self.app = app
        self.test = test
        self.error = None

    def doExecute(self):
        app_id = getAppID(self.app)
        if app_id < 0:
            self.error = 'app ' + self.app + ' not found'
            return

        test_id = getTestID(app_id, self.test)
        if test_id < 0:
            self.error = 'app ' + self.app + ' contains no such test: ' + self.test
            return

        query_result = execQuery(
            "SELECT version.name FROM app,version,test,version_test "
            "WHERE version.app_id=app.id AND test.app_id=app.id "
            "AND version_test.version_id=version.id AND version_test.test_id=test.id "
            "AND app.id=? AND test.id=?;", (app_id, test_id))
        if len(query_result) > 0:
            self.error = (
                'the test is already associated with the following version(s): ' +
                ", ".join(str(v) for v in zip(*query_result)[0]))
            return

        execQuery("DELETE FROM test WHERE app_id=? AND id=?;", (app_id, test_id))
        commit()

    def execute(self):
        self.doExecute()
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({}, sys.stdout) # ### ignoring self.error for now

    def writeOutputAsPlainText(self):
        if self.error != None:
            print 'error: ' + self.error


class RenameTest(Command):
    def __init__(self, http_get, app, old, new):
        self.http_get = http_get
        self.app = app
        self.old = old
        self.new = new
        self.error = None

    def doExecute(self):
        self.new = self.new.strip()
        if self.new == '':
            self.error = 'empty test name'
            return

        app_id = getAppID(self.app)
        if app_id < 0:
            self.error = 'app ' + self.app + ' not found'
            return

        old_id = getTestID(app_id, self.old)
        if old_id < 0:
            self.error = 'app ' + self.app + ' contains no such test: ' + self.old
            return

        new_id = getTestID(app_id, self.new)
        if new_id >= 0:
            self.error = 'app ' + self.app + ' already contains test: ' + self.new
            return

        execQuery(
            "UPDATE test SET name=? WHERE app_id=? AND id=?;",
            (unicode(self.new, 'utf-8'), app_id, old_id))
        commit()

    def execute(self):
        self.doExecute()
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({}, sys.stdout) # ### ignoring self.error for now

    def writeOutputAsPlainText(self):
        if self.error != None:
            print 'error: ' + self.error


class SetTestDescr(Command):
    def __init__(self, http_get, app, test, descr):
        self.http_get = http_get
        self.app = app
        self.test = test
        self.descr = descr
        self.error = None

    def doExecute(self):
        app_id = getAppID(self.app)
        if app_id < 0:
            self.error = 'app ' + self.app + ' not found'
            return

        test_id = getTestID(app_id, self.test)
        if test_id < 0:
            self.error = 'app ' + self.app + ' contains no such test: ' + self.test
            return

        descr_content = readTestDescription(self.descr)
        if (descr_content == ''):
            self.error = 'empty description'
            return

        execQuery(
            "UPDATE test SET description=? WHERE app_id=? AND name=?;",
            (unicode(descr_content, 'utf-8'), app_id, self.test))
        commit()

    def execute(self):
        self.doExecute()
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({}, sys.stdout) # ### ignoring self.error for now

    def writeOutputAsPlainText(self):
        if self.error != None:
            print 'error: ' + self.error


class GetTestDescr(Command):
    def __init__(self, http_get, app, test):
        self.http_get = http_get
        self.app = app
        self.test = test
        self.error = None

    def doExecute(self):
        app_id = getAppID(self.app)
        if app_id < 0:
            self.error = 'app ' + self.app + ' not found'
            return

        test_id = getTestID(app_id, self.test)
        if test_id < 0:
            self.error = 'app ' + self.app + ' contains no such test: ' + self.test
            return

        query_result = execQuery("SELECT description FROM test WHERE app_id=? AND id=?;", (app_id, test_id))
        self.descr = query_result[0][0]

    def execute(self):
        self.doExecute()
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({}, sys.stdout) # ### ignoring self.error for now

    def writeOutputAsPlainText(self):
        if self.error != None:
            print 'error: ' + self.error
        else:
            print self.descr


class GetVersionTests(Command):
    def __init__(self, http_get, app, version, test):
        self.http_get = http_get
        self.app = app
        self.version = version
        self.test = test
        self.error = None

    def doExecute(self):
        app_id = getAppID(self.app)
        if app_id < 0:
            self.error = 'app ' + self.app + ' not found'
            return
        self.versions, self.tests, dummy, self.error = getVersionTests(self.app, app_id, self.version, self.test)

    def execute(self):
        self.doExecute()
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({}, sys.stdout) # empty for now

    def writeOutputAsPlainText(self):
        if self.error != None:
            print 'error: ' + self.error
        else:
            for i in range(len(self.versions)):
                print '{:>10} {:>10}'.format(self.versions[i], self.tests[i])


class AddVersionTests(Command):
    def __init__(self, http_get, app, version, src_version, test):
        self.http_get = http_get
        self.app = app
        self.version = version
        self.src_version = src_version
        self.test = test
        self.error = None

    def doExecute(self):
        self.version = self.version.strip()
        if self.version == '':
            self.error = 'empty version name'
            return

        app_id = getAppID(self.app)
        if app_id < 0:
            self.error = 'app ' + self.app + ' not found'
            return

        version_id = getVersionID(app_id, self.version)
        if version_id < 0:
            self.error = 'app ' + self.app + ' contains no such version: ' + self.version
            return

        # compute set of tests to combine with self.version
        tests = []
        if (self.src_version == None) and (self.test == None):
            # add all tests associated with self.app (regardless of version)
            tests += getTests(app_id)
        else:
            if self.test != None:
                # add self.test
                test_id = getTestID(app_id, self.test)
                if test_id < 0:
                    self.error = 'app ' + self.app + ' contains no such test: ' + self.test
                    return
                tests.append(self.test)
            if (self.src_version != None):
                # add all tests associated with self.app/self.src_version
                src_version_id = getVersionID(app_id, self.src_version)
                if src_version_id < 0:
                    self.error = 'app ' + self.app + ' contains no such version: ' + self.src_version
                    return
                query_result = execQuery(
                    "SELECT test.name FROM app,version,test,version_test "
                    "WHERE app.id=? AND version.id=? AND version.app_id=app.id AND test.app_id=app.id "
                    "AND version_test.version_id=version.id AND version_test.test_id=test.id;",
                    (app_id, src_version_id))
                tests += ([] if (len(query_result) == 0) else zip(*query_result)[0])

        # add combinations
        for test in tests:
            test_id = getTestID(app_id, test)
            execQuery(
                "INSERT OR IGNORE INTO version_test "
                "(version_id, test_id) "
                "VALUES (?, ?)",
                (version_id, test_id))
        commit()

    def execute(self):
        self.doExecute()
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({}, sys.stdout) # ### ignoring self.error for now

    def writeOutputAsPlainText(self):
        if self.error != None:
            print 'error: ' + self.error


class RemoveVersionTests(Command):
    def __init__(self, http_get, app, version, test):
        self.http_get = http_get
        self.app = app
        self.version = version
        self.test = test
        self.error = None

    def doExecute(self):
        app_id = getAppID(self.app)
        if app_id < 0:
            self.error = 'app ' + self.app + ' not found'
            return

        dummy, dummy, version_test_ids, self.error = getVersionTests(self.app, app_id, self.version, self.test)
        if self.error != None:
            return

        # check that no test results exist for the target version/test combinations
        for id in version_test_ids:
            query_result = execQuery(
            "SELECT count(test_result.id) FROM version_test,test_result "
            "WHERE test_result.version_test_id=version_test.id "
            "AND version_test.id=?;", (id,))
            ntest_results = query_result[0][0]
            if ntest_results > 0:
                version, test = getVersionTest(id)
                if version == None:
                    version = '<### None>'
                if test == None:
                    test = '<### None>'
                self.error = (
                    'app ' + self.app + ' still contains ' + str(ntest_results) +
                    ' test result' + ('s' if (ntest_results != 1) else '') +
                    ' for version ' + version + ', test ' + test)
                return

        for id in version_test_ids:
            execQuery("DELETE FROM version_test WHERE id=?;", (id,))
        commit()

    def execute(self):
        self.doExecute()
        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({}, sys.stdout) # empty for now

    def writeOutputAsPlainText(self):
        if self.error != None:
            print 'error: ' + self.error


class GetTestResults(Command):
    def __init__(self, http_get, app, version, test):
        self.http_get = http_get
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
        zqr = zip(*self.query_result)
        if len(zqr) != 6:
            zqr = [[]] * 6
        self. ids = zqr[0]
        self.timestamps = zqr[1]
        self.reporters = zqr[2]
        self.ipaddresses = zqr[3]
        self.statuses = zqr[4]
        self.comments = zqr[5]

        self.writeOutput()

    def writeOutputAsJSON(self):
        printJSONHeader()
        json.dump({
                'ids': self.ids,
                'timestamps': self.timestamps,
                'reporters': self.reporters,
                'ipaddresses': self.ipaddresses,
                'statuses': self.statuses,
                'comments': self.comments
                }, sys.stdout)

    def writeOutputAsPlainText(self):
        for id in self.ids:
            print id


class AddTestResult(Command):
    def __init__(self, http_get, app, version, test, reporter, status, ipaddress, comment):
        self.http_get = http_get
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

    def writeOutputAsPlainText(self):
        pass


class RemoveTestResult(Command):
    def __init__(self, http_get, id):
        self.http_get = http_get
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

    def writeOutputAsPlainText(self):
        pass

# --- END Global classes ----------------------------------------------


# --- BEGIN Global functions ----------------------------------------------

# Returns the non-negative ID of the given app if it exists, otherwise a negative integer.
def getAppID(app):
    query_result = execQuery(
        "SELECT app.id FROM app WHERE name=?", (app,))
    return query_result[0][0] if (len(query_result) > 0) else -1

# Returns the non-negative ID of the given version if it exists in an app with the given ID,
# otherwise a negative integer.
def getVersionID(app_id, version):
    query_result = execQuery(
        "SELECT version.id FROM app,version WHERE app.id=version.app_id "
        "AND app.id=? AND version.name=?", (app_id, version))
    return query_result[0][0] if (len(query_result) > 0) else -1

# Returns the non-negative ID of the given test if it exists in an app with the given ID,
# otherwise a negative integer.
def getTestID(app_id, test):
    query_result = execQuery(
        "SELECT test.id FROM app,test WHERE app.id=test.app_id AND app.id=? AND test.name=?", (app_id, test))
    return query_result[0][0] if (len(query_result) > 0) else -1

# Returns versions associated with the app with the given ID.
def getVersions(app_id):
    query_result = execQuery(
        "SELECT version.name FROM app,version "
        "WHERE version.app_id=app.id AND app.id=?;", (app_id,))
    return [] if (len(query_result) == 0) else zip(*query_result)[0]

# Returns tests associated with the app with the given ID.
def getTests(app_id):
    query_result = execQuery(
        "SELECT test.name FROM app,test "
        "WHERE test.app_id=app.id AND app.id=?;", (app_id,))
    return [] if (len(query_result) == 0) else zip(*query_result)[0]

# Returns the version and test of a version test with the given ID.
def getVersionTest(id):
    query_result = execQuery(
        "SELECT version.name,test.name FROM version,test,version_test "
        "WHERE version_test.version_id=version.id "
        "AND version_test.test_id=test.id "
        "AND version_test.id=?;", (id,))
    zqr = zip(*query_result)
    if len(zqr) != 2:
        zqr = [[]] * 2
    return (None, None) if (len(query_result) == 0) else (zqr[0][0], zqr[1][0])

# Returns three values (v1, v2, v3) as follows:
#   v1: the list of version components in the version/test combinations defined for the given app,
#       or None upon error
#   v2: ditto for the test components
#   v3: ditto for the version/test IDs
#   v4: the error description, or None upon success
def getVersionTests(app, app_id, version, test):
    base_query = (
        "SELECT version.name,test.name,version_test.id FROM app,version,test,version_test "
        "WHERE app.id=? AND version.app_id=app.id AND test.app_id=app.id "
        "AND version_test.version_id=version.id AND version_test.test_id=test.id ")

    version_id = -1
    if version != None:
        version_id = getVersionID(app_id, version)
        if version_id < 0:
            error = 'app ' + app + ' contains no such version: ' + version
            return None, None, None, error

    test_id = -1
    if test != None:
        test_id = getTestID(app_id, test)
        if test_id < 0:
            error = 'app ' + app + ' contains no such test: ' + test
            return None, None, None, error

    if (version_id >= 0) and (test_id >= 0):
        query_result = execQuery(
            base_query + "AND version.id=? AND test.id=?;", (app_id, version_id, test_id))
    elif version_id >= 0:
        query_result = execQuery(base_query + "AND version.id=?;", (app_id, version_id))
    elif test_id >= 0:
        query_result = execQuery(base_query + "AND test.id=?;", (app_id, test_id))
    else:
        query_result = execQuery(base_query + ";", (app_id,))

    zqr = zip(*query_result)
    if len(zqr) != 3:
        zqr = [[]] * 3
    return zqr[0], zqr[1], zqr[2], None

# Returns a test description (primarily from a file and secondarily directly from a string).
def readTestDescription(src):
    try:
        f = open(src)
        descr = f.read()
        f.close()
    except:
        descr = src # fallback
    return descr.strip()

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
            '  --cmd add_app --app A | \\\n'
            '  --cmd remove_app --app A | \\\n'
            '  --cmd rename_app --old O --new N | \\\n'
            '  --cmd get_versions --app A | \\\n'
            '  --cmd add_version --app A --version V | \\\n'
            '  --cmd remove_version --app A --version V | \\\n'
            '  --cmd rename_version --app A --old O --new N | \\\n'
            '  --cmd get_tests --app A [--version V] | \\\n'
            '  --cmd add_test --app A --test T --descr D | \\\n'
            '  --cmd remove_test --app A --test T | \\\n'
            '  --cmd rename_test --app A --old O --new N | \\\n'
            '  --cmd set_test_descr --app A --test T --descr D | \\\n'
            '  --cmd get_test_descr --app A --test T | \\\n'
            '  --cmd add_version_tests --app A --version V [--src_version S] [--test T] | \\\n'
            '  --cmd get_version_tests --app A [--version S] [--test T] | \\\n'
            '  --cmd remove_version_tests --app A [--version S] [--test T] | \\\n'
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
        return GetApps(http_get)

    # --- 'add_app' ---------------------------------
    elif cmd == 'add_app':
        if 'app' in options:
            return AddApp(http_get, options['app'])

    # --- 'remove_app' ---------------------------------
    elif cmd == 'remove_app':
        if ('app' in options):
            return RemoveApp(http_get, options['app'])

    # --- 'rename_app' ---------------------------------
    elif cmd == 'rename_app':
        if ('old' in options) and ('new' in options):
            return RenameApp(http_get, options['old'], options['new'])

    # --- 'get_versions' ---------------------------------
    elif cmd == 'get_versions':
        if 'app' in options:
            return GetVersions(http_get, options['app'])

    # --- 'add_version' ---------------------------------
    elif cmd == 'add_version':
        if ('app' in options) and ('version' in options):
            return AddVersion(http_get, options['app'], options['version'])

    # --- 'remove_version' ---------------------------------
    elif cmd == 'remove_version':
        if ('app' in options) and ('version' in options):
            return RemoveVersion(http_get, options['app'], options['version'])

    # --- 'rename_version' ---------------------------------
    elif cmd == 'rename_version':
        if ('app' in options) and ('old' in options) and ('new' in options):
            return RenameVersion(http_get, options['app'], options['old'], options['new'])

    # --- 'get_tests' ---------------------------------
    elif cmd == 'get_tests':
        if ('app' in options):
            return GetTests(http_get, options['app'], options['version'] if 'version' in options else None)

    # --- 'add_test' ---------------------------------
    elif cmd == 'add_test':
        if ('app' in options) and ('test' in options) and ('descr' in options):
            return AddTest(http_get, options['app'], options['test'], options['descr'])

    # --- 'remove_test' ---------------------------------
    elif cmd == 'remove_test':
        if ('app' in options) and ('test' in options):
            return RemoveTest(http_get, options['app'], options['test'])

    # --- 'rename_test' ---------------------------------
    elif cmd == 'rename_test':
        if ('app' in options) and ('old' in options) and ('new' in options):
            return RenameTest(http_get, options['app'], options['old'], options['new'])

    # --- 'set_test_descr' ---------------------------------
    elif cmd == 'set_test_descr':
        if ('app' in options) and ('test' in options) and ('descr' in options):
            return SetTestDescr(http_get, options['app'], options['test'], options['descr'])

    # --- 'get_test_descr' ---------------------------------
    elif cmd == 'get_test_descr':
        if ('app' in options) and ('test' in options):
            return GetTestDescr(http_get, options['app'], options['test'])

    # --- 'get_version_tests' ---------------------------------
    elif cmd == 'get_version_tests':
        if ('app' in options):
            return GetVersionTests(
                http_get, options['app'],
                options['version'] if 'version' in options else None,
                options['test'] if 'test' in options else None)

    # --- 'add_version_tests' ---------------------------------
    elif cmd == 'add_version_tests':
        if ('app' in options) and ('version' in options):
            return AddVersionTests(
                http_get, options['app'], options['version'],
                options['src_version'] if 'src_version' in options else None,
                options['test'] if 'test' in options else None)

    # --- 'remove_version_tests' ---------------------------------
    elif cmd == 'remove_version_tests':
        if ('app' in options):
            return RemoveVersionTests(
                http_get, options['app'],
                options['version'] if 'version' in options else None,
                options['test'] if 'test' in options else None)

    # --- 'get_test_results' ---------------------------------
    elif cmd == 'get_test_results':
        if ('app' in options) and ('version' in options) and ('test' in options):
            return GetTestResults(http_get, options['app'], options['version'], options['test'])

    # --- 'add_test_result' ---------------------------------
    elif cmd == 'add_test_result':
        if (('app' in options) and ('version' in options) and ('test' in options)
            and ('reporter' in options) and ('status' in options) and ('ipaddress' in options)
            and ('comment' in options)):
            return AddTestResult(
                http_get, options['app'], options['version'], options['test'],
                options['reporter'], options['status'], options['ipaddress'],
                options['comment'])

    # --- 'remove_test_result' ---------------------------------
    elif cmd == 'remove_test_result':
        if ('id' in options):
            return RemoveTestResult(http_get, options['id'])

    # No match:
    printUsageError()
    sys.exit(1)

# --- END Global functions ----------------------------------------------


# --- BEGIN Main program ----------------------------------------------

options, http_get = getOptions()
command = createCommand(options, http_get)
command.execute()

# --- END Main program ----------------------------------------------
