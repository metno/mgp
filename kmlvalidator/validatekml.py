#!/usr/bin/python

"""
This script validates a KML file against a XML Schema.
"""

import sys, os
from lxml import etree
from xml.dom.minidom import parseString
from traceback import format_exc

# --- BEGIN global functions ---------------------------------------

def validateKML(kml_str, schema_fname):
    # parse KML document
    try:
        kml_doc = etree.fromstring(kml_str)
    except:
        raise Exception, 'failed to parse KML document: {}'.format(format_exc())

    # parse schema document
    try:
        schema_doc = etree.parse(schema_fname)
    except:
        raise Exception, 'failed to parse schema {}: {}'.format(schema_fname, format_exc())

    # create schema
    try:
        schema = etree.XMLSchema(schema_doc)
    except:
        raise Exception, 'failed to create schema object from {}: {}'.format(schema_fname, format_exc())

    # validate KML document against schema
    try:
        schema.assertValid(kml_doc)
    except etree.DocumentInvalid as e:
        #sys.stderr.write('error log: {}\n'.format(str(schema.error_log)))
        raise Exception, 'KML document is invalid according to XML Schema {}: {}\n'.format(schema_fname, e)
    except:
        raise Exception, 'other exception: {}'.format(format_exc())

def printUsage():
    sys.stderr.write('usage: ' + sys.argv[0] + ' <KML file> <schema file>\n')

# --- END global functions ---------------------------------------


# --- BEGIN main program ---------------------------------------

if len(sys.argv) != 3:
    printUsage()
    sys.exit(1)
kml_fname = sys.argv[1]
schema_fname = sys.argv[2]

f = open(kml_fname)
kml = f.read()
f.close()

try:
    validateKML(kml, schema_fname)
except Exception:
    sys.stderr.write('failed to validate KML file: {}\n'.format(format_exc()))
    sys.exit(1)


sys.stderr.write('no validation errors found\n')
sys.exit(0)

# --- END main program ---------------------------------------
