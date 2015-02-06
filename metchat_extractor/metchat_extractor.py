#!/usr/bin/python

"""
This program merges together multiple MetChat database files (in SQLite format) and prints chat messages
within a specified time interval.

Example:
1412121600 (2014-10-01)
1417305600 (+ 60 days)

./metchat_extractor.py 1412121600 1417305600 ~/metchat_backup/*.db

"""

import sys, os
from datetime import datetime
import sqlite3

if len(sys.argv) < 4:
    sys.stderr.write('usage: ' + sys.argv[0] + ' <from timestamp> <to timestamp> file1 [file2 ...]\n')
    sys.exit(1)

timestamp_from = sys.argv[1]
timestamp_to = sys.argv[2]

channel_id = 1 # for now

msg = {}

# process input files
for fname in sys.argv[3:]:
    con = sqlite3.connect(fname)
    rows = con.execute(
        'SELECT id,timestamp,user,text FROM log WHERE timestamp >= ? AND timestamp <= ? ORDER BY timestamp;',
        (timestamp_from, timestamp_to))

    msg_count = dup_count = 0
    for row in rows:
        msg_count =msg_count + 1
        id, timestamp, user, text = row[:]
        m = [timestamp, user, text]
        if id in msg:
            dup_count = dup_count + 1
            if m != msg[id]:
                sys.stderr.write('error: different contents found for same id\n')
                sys.exit(1)
        else:
            msg[id] = m

    #print 'file:', fname, ', messages:', msg_count, ', duplicates:', dup_count


# write output in HTML format
sys.stdout.write('<html><head><meta http-equiv="content-type" content="text/html;charset=utf-8" /></head><body><pre>\n')
sorted_msg = sorted(msg.items(), key=lambda e: e[1][0]) # sort on timestamp
for m in sorted_msg:
    m1 = m[1]
    sys.stdout.write('{} {:10} {}\n'.format(
            datetime.fromtimestamp(m1[0]).strftime('%Y-%m-%d %H:%M:%S'), m1[1], m1[2].encode('utf-8')))
sys.stdout.write('</pre></body></html>\n')

sys.exit(0)
