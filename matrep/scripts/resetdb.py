#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
This script resets the MATRep SQLite database file.
"""

import sys, os, sqlite3

# --- BEGIN global functions ---------------------------------------

def setTestData(conn):
    # create apps
    for app in ['diana', 'ted', 'tseries', 'modfly']: # assume id=1..4
        conn.execute("INSERT INTO app (name) VALUES (?);", (app,))

    # create versions for diana
    app_id_diana = 1 # by assumption
    for version in ['3.33', '3.3', '3.35']: # assume id=1..3
        conn.execute("INSERT INTO version (app_id, name) VALUES (?, ?);", (app_id_diana, version))

    # create tests for diana
    app_id_diana = 1 # by assumption
    conn.execute("INSERT INTO test (app_id, name, description) VALUES (?, ?, ?);",
                 (app_id_diana, 'diana-test1',
                  """
                  Denne Diana-testen (diana-test1) består i å utføre følgende steg:
                  <ol>
                  <li> gjør ditt
                  <li> gjør <b>datt</b>
                  <li> sjekk at ...
                  </ol>
                  """.decode('utf8').strip())
                 ) # assume id=1
    conn.execute("INSERT INTO test (app_id, name, description) VALUES (?, ?, ?);",
                 (app_id_diana, 'diana-test2',
                  """
                  Denne Diana-testen (diana-test2) består i å utføre følgende steg:
                  <ol>
                  <li> gjør ditt
                  <li> gjør <b>datt 0</b>
                  <li> gjør datt 1
                  <li> gjør datt 2
                  <li> gjør datt 3
                  <li> gjør datt 4
                  <li> gjør datt 5
                  <li> gjør datt 6
                  <li> gjør datt 7
                  <li> gjør datt 8
                  <li> gjør datt 9
                  <li> gjør datt 10
                  <li> gjør datt 11
                  <li> gjør datt 12
                  <li> gjør datt 13
                  <li> gjør datt 14
                  <li> gjør datt 15
                  <li> gjør datt 16
                  <li> gjør datt 17
                  <li> gjør datt 18
                  <li> gjør datt 19
                  <li> gjør datt 20
                  <li> gjør datt 21
                  <li> gjør datt 22
                  <li> gjør datt 23
                  <li> gjør datt 24
                  <li> gjør datt 25
                  <li> sjekk at ...
                  </ol>
                  """.decode('utf8').strip())
                 ) # assume id=2

    # create version/test combinations for diana
    # note:
    # - test2 (id=2) applies to version 3.35 (id=3) only)
    # - there are no tests for version 3.3 (id=2)
    combs = [[1, 1], [3, 1], [3, 2]]
    for c in combs: # assume id=1..3
        conn.execute("INSERT INTO version_test (version_id, test_id) VALUES (?, ?);", (c[0], c[1]))

    # create versions for modfly
    app_id_modfly = 4 # by assumption
    for version in ['4.8', '4.9']: # assume id=4..5
        conn.execute("INSERT INTO version (app_id, name) VALUES (?, ?);", (app_id_modfly, version))

    # create tests for modfly
    app_id_modfly = 4 # by assumption
    conn.execute("INSERT INTO test (app_id, name, description) VALUES (?, ?, ?);",
                 (app_id_modfly, 'modfly-test1',
                  """
                  Denne Modfly-testen (modfly-test1) består i å utføre følgende steg:
                  <ol>
                  <li> gjør ditt
                  <li> gjør <b>datt</b>
                  <li> sjekk at ...
                  </ol>
                  """.decode('utf8').strip())
                 ) # assume id=3
    for i in [2, 3, 4, 5, 6, 7, 8, 9, 10, 11]: # assume id=4..13
        conn.execute("INSERT INTO test (app_id, name, description) VALUES (?, ?, ?);",
                     (app_id_modfly, 'modfly-test{}'.format(i), 'modfly-test{} ...'.format(i)))

    # create version/test combinations for modfly
    combs = [[4, 3], [5, 3], [5, 4], [5, 5], [5, 6], [5, 7], [5, 8], [5, 9],
             [5, 10], [5, 11], [5, 12], [5, 13]] # assume id=4..5
    for c in combs:
        conn.execute("INSERT INTO version_test (version_id, test_id) VALUES (?, ?);", (c[0], c[1]))


    # create two test results for diana/3.33/diana-test-1
    conn.execute(
        "INSERT INTO test_result "
        "(version_test_id, timestamp, reporter, ipaddress, status, comment) "
        "VALUES (?, ?, ?, ?, ?, ?);",
        (1, 1398422318, 'joa', '157.249.115.120', 'fail', u'testen feilet pga æøå ...'))
    conn.execute(
        "INSERT INTO test_result "
        "(version_test_id, timestamp, reporter, ipaddress, status, comment) "
        "VALUES (?, ?, ?, ?, ?, ?);",
        (1, 1398422328, 'juergen', '157.249.115.122', 'pass', ''))

def resetDatabase(fname, set_test_data):
    if os.path.exists(fname):
        sys.stderr.write('error: file \'{}\' already exists\n'.format(fname))
        sys.exit(1)

    conn = sqlite3.connect(fname)
    #conn.row_factory = sqlite3.Row # for accessing rows by name as well as by index

    conn.execute('pragma foreign_keys=ON')
    rows = conn.execute('pragma foreign_keys')
    for row in rows:
       if not row:
           sys.stderr.write('warning: failed to enable foreign key support\n')

    conn.execute("""
        CREATE TABLE app(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            UNIQUE(name)
        );
    """)

    conn.execute("""
        CREATE TABLE version(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            app_id INTEGER,
            name TEXT NOT NULL,
            FOREIGN KEY(app_id) REFERENCES app(id),
            UNIQUE(app_id, name)
        );
    """)

    conn.execute("""
        CREATE TABLE test(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            app_id INTEGER,
            name TEXT NOT NULL,
            description TEXT NOT NULL,
            FOREIGN KEY(app_id) REFERENCES app(id),
            UNIQUE(app_id, name)
        );
    """)

    conn.execute("""
        CREATE TABLE version_test(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            version_id INTEGER,
            test_id INTEGER,
            FOREIGN KEY(version_id) REFERENCES version(id),
            FOREIGN KEY(test_id) REFERENCES test(id),
            UNIQUE(version_id, test_id)
        );
    """)

    conn.execute("""
        CREATE TABLE test_result(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            version_test_id INTEGER,
            timestamp INTEGER NOT NULL,
            reporter TEXT NOT NULL,
            ipaddress TEXT NOT NULL,
            status TEXT NOT NULL,
            comment TEXT NOT NULL,
            FOREIGN KEY(version_test_id) REFERENCES version_test(id)
        );
    """)

    if set_test_data:
        setTestData(conn)

    conn.commit()

    sys.stdout.write('done\n')

# --- END global functions ---------------------------------------


# --- BEGIN main program ---------------------------------------

if 'SQLITEFILE' in os.environ:
    resetDatabase(
        os.environ['SQLITEFILE'],
        ('SETTESTDATA' in os.environ) and (os.environ['SETTESTDATA'].lower() not in ['0', 'false', 'no']))
else:
    sys.stderr.write('error: environment variable SQLITEFILE not set\n')
    sys.exit(1)

sys.exit(0)

# --- END main program ---------------------------------------
