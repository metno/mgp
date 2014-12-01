#!/usr/bin/env python

import gearman
import time

def reversed(s):
    return s[::-1]

def task_listener_reverse(worker, job):
    #time.sleep(10)
    return reversed(job.data)

worker = gearman.GearmanWorker(['localhost'])
worker.register_task('reverse', task_listener_reverse)

worker.work()
