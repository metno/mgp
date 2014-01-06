#!/usr/bin/env python

import socket, sys

# On pc4131:
#host = '127.0.0.1' # ok
#host = 'metchat' # hangs
#host = 'metchat.met.no' # hangs
#host = 'dev-vm075.met.no' # hangs
#host = '157.249.66.75' # hangs

if len(sys.argv) != 3:
    print 'usage: {} <TCP server host> <TCP server port>'.format(sys.argv[0])
    sys.exit(1)

host = sys.argv[1]
port = int(sys.argv[2])

bufsize = 1024
msg = 'Hello, World!'

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host, port))
s.send(msg)
data = s.recv(bufsize)
s.close()

print 'received data:', data
