# -*- coding: utf-8 -*-
"""
@author: msquirogac
"""
import socket
import slink

HOST, PORT = "169.254.1.1", 5000
HOST, PORT = "127.0.0.1", 5000
addr = (HOST, PORT)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("", PORT+1))

messageTX = slink.slink()
messageTX.InitMessage()
messageTX.SetIdentifier(0x1B)
messageTX.LoadPayload(bytearray([51,52,53,54]))
messageTX.EndMessage()

data = messageTX.Packet
sock.sendto(data, addr)

data = messageTX.Packet + bytes([0,0,0,0])
messageRX = slink.slink()
result, data = messageRX.ReceiveMessage(data)

sock.close()

