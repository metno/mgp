#!/usr/bin/python

"""
This program prints statistics for a set of MetChat database files (in SQLite format).

Example:

  ./metchat_stats.py ~/metchat_backup/*.db

"""

import sys, os
from datetime import datetime
import sqlite3

normal_channel_id = 1 # for now

# --- BEGIN Global functions ----------------------------------------------

def printUsage():
    sys.stderr.write('usage: {} usr_msg_count|day_msg_count file1 [file2 ...]\n')

# Returns a dictionary that contains the unique messages found in a set of files.
# If cid != None, only messages from the channel with this ID are included.
def loadUniqueMessages(fileNames, cid = None):
    msg = {}
    for fname in fileNames:
        con = sqlite3.connect(fname)
        rows = con.execute('SELECT id,timestamp,user,text,channelId FROM log ORDER BY timestamp;')

        for row in rows:
            id, timestamp, user, text, channel_id = row[:]
            if (cid == None) or (cid == channel_id):
                m = [timestamp, user, text, channel_id]
                if id in msg:
                    # ignore occurrence but ensure that it is identical to the original one
                    if m != msg[id]:
                        sys.stderr.write('error: different contents found for same id\n')
                        sys.exit(1)
                else:
                    # record first occurrence
                    msg[id] = m

    return msg

# Prints the per-user message count, ordered from high to low count.
def printUsrMsgCount(msg):

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

# Prints the per-day message count, ordered chronologically.
def printDayMsgCount(msg, human = True):

    secs_in_day = 86400
    day_count = {}

    # compute per-day message count
    for m in msg.values():
        tstamp = int(m[0])
        day = tstamp - tstamp % secs_in_day
        if not day in day_count:
            day_count[day] = 0
        day_count[day] = day_count[day] + 1

    # write stats to stdout
    if human:
        sys.stdout.write('*** per-day message count: ***\n')
    day = min(day_count.keys())
    max_day = max(day_count.keys())
    while day <= max_day:
        dt = datetime.fromtimestamp(day)
        dc = day_count[day] if day in day_count else 0
        if human:
            sys.stdout.write('{}{}: {:6}\n'.format(
                    '\n' if (dt.weekday() == 0) else '',
                    dt.strftime('%Y-%m-%d (%a)'), dc))
        else:
            sys.stdout.write('{} {:6}\n'.format(day, dc))
        day = day + secs_in_day

# --- END Global functions ----------------------------------------------


# --- BEGIN Main program ----------------------------------------------

if len(sys.argv) < 3:
    printUsage()
    sys.exit(1)

#channel_id = None # any channel (i.e. channel filter disabled)
channel_id = 1 # salen
#channel_id = 4 # test
#channel_id = 5 # diskusjon
#channel_id = 6 # bora
msg = loadUniqueMessages(sys.argv[2:], channel_id)

#human = True # Human readable output
human = False # Output suitable for input to plotters (xgraph, gnuplot)

if sys.argv[1] == 'usr_msg_count':
    printUsrMsgCount(msg)
elif sys.argv[1] == 'day_msg_count':
    printDayMsgCount(msg, human)
else:
    printUsage()
    sys.exit(1)

sys.exit(0)

# --- END Main program ----------------------------------------------
