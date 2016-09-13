# -*- coding: utf-8 -*-
"""
@author: msquirogac
"""
import slink
import time

cycles = 100000
payload = bytes([0 for x in range(100)])
packetpad = bytes([0,0,0,0])
print('Cycles: {} PayloadSize: {} PadSize: {}'.format(cycles, len(payload), len(packetpad)))

messageTX = slink.slink()
messageRX = slink.slink()

# Message TX
count = 0
t1 = time.time()
for i in range(0, cycles):
    messageTX.InitMessage()
    messageTX.SetIdentifier(0x1B)
    messageTX.LoadPayload(payload)
    messageTX.EndMessage()
    count += 1
t2 = time.time()
dt = (t2-t1)*1000
print('Time: {:.3f} Count: {}'.format(dt, count))

packet = messageTX.Packet + packetpad

# Message RX
count = 0
t1 = time.time()
for i in range(0, cycles):
    buffer = packet
    while len(buffer):
        result, buffer = messageRX.ReceiveMessage(buffer)
        if result == 1:
            count += 1
t2 = time.time()
dt = (t2-t1)*1000
print('Time: {:.3f} Count: {}'.format(dt, count))
