#!/bin/sh

# This script runs a smoke test of WMS/Diana.

tstdir=`dirname $0`
$tstdir/smoketest.py --schema $tstdir/capabilities_1_3_0_2.xsd --schemaurls \
'https://git.met.no/cgi-bin/gitweb.cgi?p=joa.git;a=blob_plain;f=wms_test/capabilities_1_3_0_2.xsd;hb=HEAD
 http://schemas.opengis.net/wms/1.3.0/capabilities_1_3_0.xsd

 https://git.met.no/cgi-bin/gitweb.cgi?p=joa.git;a=blob_plain;f=wms_test/sld_capabilities_1_1_0_2.xsd;hb=HEAD
 http://schemas.opengis.net/sld/1.1.0/sld_capabilities.xsd

 https://git.met.no/cgi-bin/gitweb.cgi?p=joa.git;a=blob_plain;f=wms_test/xlink.xsd;hb=HEAD
 http://www.w3.org/1999/xlink.xsd

 https://git.met.no/cgi-bin/gitweb.cgi?p=joa.git;a=blob_plain;f=wms_test/xml.xsd;hb=HEAD
 http://www.w3.org/2001/xml.xsd
'
