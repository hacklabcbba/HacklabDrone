# -*- coding: utf-8 -*-
"""
@author: msquirogac
"""

class crc32(object):
    __slots__ = ['lut']
    def __init__(self):
        self.lut = self.GenerateLut(0xEDB88320)

    @staticmethod
    def GenerateLut(polynomial):
        output = []
        for ii in range(256):
            elem = ii
            for j in range(8):
                shift = elem >> 1
                elem = shift ^ (elem & 1) * polynomial
            output.append(elem & 0xFFFFFFFF)
        return tuple(output);

    def Calculate(self, crc, data):
        length = len(data)
        crc = (crc ^ 0xFFFFFFFF)
        for ii in range(length):
            crc = crc ^ data[ii]
            crc = (crc >> 8) ^ (self.lut[crc & 0xFF])
        return (crc ^ 0xFFFFFFFF)
