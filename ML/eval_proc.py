#!/usr/bin/env python3

import socket

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 2325        # Port to listen on (non-privileged ports are > 1023)
i = 0
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    conn, addr = s.accept()
    with conn:
        print('Connected by', addr)
        while True:
            data = conn.recv(8)
            num = int(data)
            print (num)
            if not data:
                break
            if i == 0:
            	conn.sendall(b"0")
            	i = 1
            else:
            	conn.sendall(b"1")
            	i = 0
