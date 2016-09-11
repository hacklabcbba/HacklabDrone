# -*- coding: utf-8 -*-
"""
@author: msquirogac
"""
import slink
import time

payload = bytearray([x+48 for x in range(84)])

messageTX = slink.slink()
messageRX = slink.slink()

# Message TX
count = 0
t1 = time.time()
for i in range(0, 10000):
    messageTX.InitMessage()
    messageTX.SetIdentifier(0x1B)
    messageTX.LoadPayload(payload)
    messageTX.EndMessage()
    count += 1
t2 = time.time()
dt = (t2-t1)*1000
print('Time: {:.3f} Count: {}'.format(dt,count))

packet = messageTX.Packet + bytes([0,0,0,0])

# Message RX
count = 0
t1 = time.time()
for i in range(0, 10000):
    buffer = packet
    while len(buffer):
        result, buffer = messageRX.ReceiveMessage(buffer)
        if result == 1:
            count += 1
t2 = time.time()
dt = (t2-t1)*1000
print('Time: {:.3f} Count: {}'.format(dt,count))
