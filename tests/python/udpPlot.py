# -*- coding: utf-8 -*-
"""
@author: msquirogac
"""
import socket
import select
import slink
import time

HOST, PORT = "169.254.1.1", 5000
#HOST, PORT = "127.0.0.1", 5000

print('Info: IP Address  = {}'.format(HOST))
print('Info: Port number = {}'.format(PORT))

remoteAddress = (HOST, PORT)
# Create Datagram Socket (UDP)
# Make Socket Reusable
# Set socket to non-blocking mode
# Accept Connections on port+1
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.setblocking(False)
sock.bind(('', 0))

# Sockets from which we expect to read
# Sockets to which we expect to write
inputs = [sock]
outputs = [sock]

messageTX = slink.slink()
messageRX = slink.slink()
messageTX.InitMessage()
messageTX.SetIdentifier(0x00)
messageTX.EndMessage()

RecvStats = type('X', (object,), dict(PacketCount=0, PacketError=0 ,PacketRate=0))
RecvCount = 0;

timeS = time.time()
flag = False
while True:
    try:
        timeL = time.time()
        if timeL - timeS >= 1:
            RecvStats.PacketRate = RecvStats.PacketCount - RecvCount;
            RecvCount = RecvStats.PacketCount;
            print('Recv rate: {} count: {} error: {}'.format(RecvStats.PacketRate, RecvStats.PacketCount, RecvStats.PacketError))
            timeS = timeL
            flag = True

        readable, writable, exceptional = select.select(inputs, outputs, inputs)
        for r in readable:
            if r is sock:
                message, address = sock.recvfrom(8192)
                if address == remoteAddress:
                    while len(message):
                        result, message = messageRX.ReceiveMessage(message)
                        if result == 1:
                            identifier = messageRX.GetIdentifier()
                            payload = messageRX.GetPayload()
                            RecvStats.PacketCount += 1
                        else:
                            RecvStats.PacketError += 1

        for w in writable:
            if w is sock:
                if flag:
                    message = messageTX.Packet
                    sock.sendto(message, remoteAddress)
                    flag = False

        time.sleep(0.0001)
    except:
        break

sock.close()
print("Exit program")
