#!/usr/bin/python

"""
This program exits with zero (i.e. success) iff the newest file among the given input files is older
than the specified number of days.

Notes:

  - The age of the file is determined from the name, not the actual modification time (which may
    have changed since the file was created).

  - The input files are assumed to be of the form <prefix>yyyy-mm-dd-qc.db
    For example: /home/joa/metchat_backup/2015-01-24-qc.db
"""

import sys, os, re, time, datetime

if len(sys.argv) < 2:
    sys.stderr.write('usage: ' + sys.argv[0] + ' <max days tolerance> [file1 file2 ...]\n')
    sys.exit(1)

try:
    max_days = int(sys.argv[1])
except:
    sys.stderr.write('failed to parse max days tolerance as integer\n')
    sys.exit(1)

# process input files and find newest timestamp
newest_ts = -1
p = re.compile('.*(\d\d\d\d)-(\d\d)-(\d\d)-qc.db$')
for fname in sys.argv[2:]:
    m = p.match(fname)
    if m:
        dt = datetime.datetime(year = int(m.group(1)), month = int(m.group(2)), day = int(m.group(3)))
        newest_ts = max(newest_ts, time.mktime(dt.timetuple()))

curr_ts = time.time()
max_secs = 24 * 3600 * max_days

if curr_ts - newest_ts > max_secs:
    sys.exit(0) # newest file is too old

sys.exit(1)
