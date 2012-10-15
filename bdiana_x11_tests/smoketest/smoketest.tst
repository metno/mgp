#!/bin/sh

# This script runs a smoke test of the X11-dependent bdiana version.

tstdir=`dirname $0`
$tstdir/smoketest.py --cmd $BDIANA_EXEC_PATH
