#!/usr/bin/python

"""
This program prints statistics for a set of MetChat database files (in SQLite format).

Example:

  ./metchat_stats.py ~/metchat_backup/*.db

"""

import sys, os
from datetime import datetime
import sqlite3

if len(sys.argv) < 2:
    sys.stderr.write('usage: ' + sys.argv[0] + ' file1 [file2 ...]\n')
    sys.exit(1)

normal_channel_id = 1 # for now

# load unique messages
msg = {}
for fname in sys.argv[3:]:
    con = sqlite3.connect(fname)
    rows = con.execute('SELECT id,timestamp,user,text,channelId FROM log ORDER BY timestamp;')

    for row in rows:
        id, timestamp, user, text, channel_id = row[:]
        m = [timestamp, user, text, channel_id]
        if id in msg:
            # ignore occurrence but ensure that it is identical to the original one
            if m != msg[id]:
                sys.stderr.write('error: different contents found for same id\n')
                sys.exit(1)
        else:
            # record first occurrence
            msg[id] = m

# compute per-user message count
user_count = {}
for m in msg.values():
    user = m[1]
    if not user in user_count:
        user_count[user] = 0
    user_count[user] = user_count[user] + 1

# write stats to stdout
sys.stdout.write('*** per-user message count: ***\n')
sorted_user_count = sorted(user_count.items(), key=lambda x: x[1], reverse=True)
for item in sorted_user_count:
    sys.stdout.write('{:6} {}\n'.format(item[1], item[0]))

sys.exit(0)
