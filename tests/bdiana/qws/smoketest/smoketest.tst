#!/bin/sh

# This script runs a smoke test of the bdiana version that depends on QWS (rather than on X11).

tstdir=`dirname $0`
$tstdir/../../shared/smoketest.py --cmd "$BDIANA_EXEC_PATH -display dummy:7771"
