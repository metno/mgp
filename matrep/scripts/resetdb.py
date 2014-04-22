#!/usr/bin/python

"""
This script resets the MATRep SQLite database file.
"""

import sys, os, sqlite3

# --- BEGIN global functions ---------------------------------------

def resetDatabase(fname):
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
            description TEXT NOT NULL,
            FOREIGN KEY(version_test_id) REFERENCES version_test(id)
        );
    """)

    sys.stdout.write('done\n')

# --- END global functions ---------------------------------------


# --- BEGIN main program ---------------------------------------

if 'SQLITEFILE' in os.environ:
    resetDatabase(os.environ['SQLITEFILE'])
else:
    sys.stderr.write('error: environment variable SQLITEFILE not set\n')
    sys.exit(1)

sys.exit(0)

# --- END main program ---------------------------------------
