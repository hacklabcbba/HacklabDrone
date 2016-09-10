# -*- coding: utf-8 -*-
"""
@author: msquirogac
"""

import socket
from array import *


HOST, PORT = "169.254.1.1", 5000
addr = (HOST, PORT)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("", PORT+1))

#dataRaw = "0123456"
#data = array('B', dataRaw.encode())
#sock.sendto(data.tostring(), ('169.254.1.1', PORT))

data = bytes([0] * 10)
sock.sendto(data, ('169.254.1.1', PORT))

data = bytearray([0] * 10)
sock.sendto(data, ('169.254.1.1', PORT))

sock.close()