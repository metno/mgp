#!/usr/bin/python

"""
This lists Jenkins jobs along with selected attributes.
"""

import sys, os, re
sys.path.append('{}/../shared/python'.format(os.path.dirname(os.path.abspath(__file__))))
from misc import getOptDict
from xml.dom.minidom import parse
from traceback import format_exc

# Validate command-line
options = getOptDict()
if not 'jenkinshome' in options:
    sys.stderr.write('usage: ' + sys.argv[0] + ' --jenkinshome <Jenkins home directory>\n')
    sys.exit(1)

attrs = [] # Attribute dictionary for each job

# Extract attributes
jobs_root_dir = '{}/jobs'.format(options['jenkinshome'])
job_names = [
    name for name in os.listdir(jobs_root_dir)
    if os.path.isdir(os.path.join(jobs_root_dir, namae))]
for job_name in job_names:
    try:
        fname = '{}/config.xml'.format(os.path.join(jobs_root_dir, job_name))
        dom = parse(fname)

        # Name
        attr = { 'name': job_name }

        # 'Assigned node'
        assignedNode_elems = dom.getElementsByTagName('assignedNode')
        if len(assignedNode_elems) == 0:
            attr['assigned_node'] = None
        else:
            assert len(assignedNode_elems[0].childNodes) > 0
            attr['assigned_node'] = assignedNode_elems[0].childNodes[0].nodeValue

        # 'Can roam'
        canRoam_elems = dom.getElementsByTagName('canRoam')
        if len(canRoam_elems) == 0:
            attr['can_roam'] = 'False'
        else:
            assert len(canRoam_elems[0].childNodes) > 0
            attr['can_roam'] = canRoam_elems[0].childNodes[0].nodeValue

        # ---------
        attrs.append(attr)

    except:
        sys.stderr.write('error: {}\n'.format(format_exc()))
        sys.exit(1)

# Print result
sorted_attrs = sorted(attrs, key=lambda x: x['assigned_node'], reverse=True)
format_s = '{:<25}  {:<25}  {:<6}\n'
sys.stdout.write(format_s.format('JOB', 'PRIMARY NODE', 'REQUIRED NODE'))
sys.stdout.write('{}\n'.format('-'*70))
for attr in sorted_attrs:
    sys.stdout.write(format_s.format(
            attr['name'],
            '' if not attr['assigned_node'] else attr['assigned_node'],
            'yes' if attr['can_roam'].lower() in ['false', 'no', '0'] else ''))
