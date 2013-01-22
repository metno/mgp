#!/bin/sh

# This script runs a smoke test of the bdiana version that depends on X11.

tstdir=`dirname $0`
$tstdir/../../shared/smoketest.py --cmd $BDIANA_EXEC_PATH
