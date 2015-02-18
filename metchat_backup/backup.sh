#!/bin/sh

srcdir=/var/lib/qc
dstdir=/home/joa/metchat_backup
dbfile=qc.db

# copy current DB file from server
rsync -a root@metchat:$srcdir/$dbfile $dstdir

# archive current DB file if more than 15 days have passed since the last archived file
# (note: we assume that the DB file contains messages 30 days back, so archiving around
# twice a month should be safe, i.e. provide plenty of overlap!)
scriptdir=`dirname "$0"`
if $scriptdir/newest_file_too_old.py 15 $dstdir/*-qc.db; then
  cp $dstdir/$dbfile $dstdir/`date "+%Y-%m-%d"`-$dbfile
fi

# copy images from server
rsync -a root@metchat:$srcdir/images/ $dstdir/images
