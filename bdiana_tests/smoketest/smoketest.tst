#!/bin/sh

# This script runs a smoke test of the headless bdiana version, i.e. the
# one that depends on QWS rather than on X11.

tstdir=`dirname $0`
$tstdir/smoketest.py --cmd "$BDIANA_EXEC_PATH -display dummy:7771"
