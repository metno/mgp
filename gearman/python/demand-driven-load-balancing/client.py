#!/usr/bin/env python

import gearman
import time

# Algorithm:

# STEP 1: Add all jobs to the INITIAL list. Let the PENDING and FINISHED lists be empty.
# STEP 2: Submit requests for the first n jobs - where n is the current number of workers -
#         and move these requests from INITIAL to PENDING.
# STEP 3: Check each request in PENDING at regular intervals (1 sec?):
#
#           Case 1: complete:
#                    assert (request in PENDING or FINISHED)
#                    if request in PENDING:
#                        move from PENDING to FINISHED
#                        if INITIAL and PENDING both empty:
#                            report total result and exit program successfully
#
#                    # At this point we know that there's demand for more work (at least by
#                    # the worker that just returned a result!), so ...
#                    if INITIAL empty:
#                        j = first job in INITIAL
#                        move j from INITIAL to PENDING
#                    else:
#                        j = next job in PENDING (in circular fashion; note that this job has
#                        already been assigned to one or more other workers, but these are either
#                        slow or have crashed)
#                    submit a request for j
#
#           Case 2: timeout
#                   if overall timeout is reached:
#                       exit program with an error (flagging timeout)
#
#           Case 3: disconnection
#                   exit program with an error (flagging disconnection)



# def check_request_status(job_request):
#     if job_request.complete:
#         print 'Job %s finished!  Result: %s - %s' % (job_request.job.unique, job_request.state, job_request.result)
#     elif job_request.timed_out:
#         print 'Job %s timed out!' % job_request.job.unique
#     elif job_request.state == JOB_UNKNOWN:
#         print 'Job %s connection failed!' % job_request.job.unique




#=============================================================================================

# initialize tasks
tasks = 


client = gearman.GearmanClient(['localhost'])

# job_request = client.submit_job('reverse', 'hello world!')
# check_request_status(job_request)

print 'done'
