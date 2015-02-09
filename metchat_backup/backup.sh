#!/bin/sh

srcdir=/var/lib/qc
dstdir=/home/joa/metchat_backup
dbfile=qc.db

# copy current DB file from server
rsync -a root@metchat:$srcdir/$dbfile $dstdir

# if day of month is 1 or 15, copy current file to archived file
# (note: we assume that the DB file contains messages 30 days back, so archiving twice
# a month should be safe (i.e. provide plenty of overlap!))
day=`date "+%d"`
if [ $day -eq 1 ] || [ $day -eq 15 ]; then
  cp $dstdir/$dbfile $dstdir/`date "+%Y-%m-%d"`-$dbfile
fi

# copy images from server
rsync -a root@metchat:$srcdir/images/ $dstdir/images
