#!/usr/bin/env python

import gearman
import time

def check_request_status(job_request):
    if job_request.complete:
        print 'Job %s finished!  Result: %s - %s' % (job_request.job.unique, job_request.state, job_request.result)
    elif job_request.timed_out:
        print 'Job %s timed out!' % job_request.job.unique
    elif job_request.state == JOB_UNKNOWN:
        print 'Job %s connection failed!' % job_request.job.unique

client = gearman.GearmanClient(['localhost'])

job_request = client.submit_job('reverse', 'hello world!')
check_request_status(job_request)

print 'done'
