#!/usr/bin/env python

import socket, sys

if len(sys.argv) != 2:
    print 'usage: {} <TCP server listen port>'.format(sys.argv[0])
    sys.exit(1)

port = int(sys.argv[1])

host = '127.0.0.1'
bufsize = 20  # Normally 1024, but we want fast response

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((host, port))
s.listen(1)

conn, addr = s.accept()
print 'connection address:', addr
while 1:
    data = conn.recv(bufsize)
    if not data: break
    print "received data:", data
    conn.send(data)  # echo
conn.close()
