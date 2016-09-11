from struct import pack, unpack
from crc32 import crc32

class slink(object):
    LENGTH_MAX = 1000
    INIT_CHECKSUM = 0xFFFFFFFF
    SOP = bytes([0x73, 0x6c])
    PREAMBLE = SOP + bytes([255, 255])

    def __init__(self):
        self.__crc = crc32()
        self.__Context = type('Context', (), {})
        self.__Context.Control = type('Control', (), {})
        self.InitMessage()

    def InitMessage(self):
        self.__Context.Control.State = 0
        self.__Context.Control.DataIdx = 0
        self.__Context.Identifier = 0
        self.__Context.Length = 0
        self.Identifier = 0
        self.Payload = bytearray()
        self.Packet = bytearray()

    def EndMessage(self):
        self.__Context.Control.State = 0
        packet = bytearray()
        packet.extend(self.PREAMBLE)
        packet.extend(pack('2H', len(self.Payload), self.Identifier))
        packet.extend(self.Payload)
        checksum = self.__crc.Calculate(self.INIT_CHECKSUM, packet[4:]) ^ 0xFFFFFFFF
        packet.extend(pack('I', checksum))
        self.Packet = packet

    def GetIdentifier(self):
        return self.Identifier

    def SetIdentifier(self, identifier):
        assert(isinstance(identifier, int))
        self.Identifier = identifier

    def LoadPayload(self, payload):
        assert((isinstance(payload, bytes) | isinstance(payload, bytearray)))
        self.Payload.extend(payload)

    def GetPayload(self):
        return self.Payload

    def GetPacket(self):
        return self.Packet

    def ReceiveMessage(self, data):
        assert((isinstance(data, bytes) | isinstance(data, bytearray)))
        result, rest = 0, bytearray([])
        packet = self.Packet
        state = self.__Context.Control.State
        dataIdx = self.__Context.Control.DataIdx
        length = self.__Context.Length
        identifier = self.__Context.Identifier
        for ii in range(len(data)):
            newByte = data[ii]
            if state == 0:
                # Sop byte 1
                if newByte == self.SOP[0]:
                    packet = bytearray([0 for x in range(8)])
                    packet[0] = newByte
                    state = 1
            elif state == 1:
                # Sop byte 2
                if newByte == self.SOP[1]:
                    packet[1] = newByte
                    state = 2
                else:
                    result, rest = -1, data[ii+1:]
                    state = 1
                    break
            elif state == 2:
                # Reserved byte 1
                packet[2] = newByte
                state = 3
            elif state == 3:
                # Reserved byte 2
                packet[3] = newByte
                state = 4
            elif state == 4:
                # Length byte 1
                packet[4] = newByte
                state = 5
            elif state == 5:
                # Length byte 2
                packet[5] = newByte
                length = unpack('H', packet[4:5+1])[0]
                if length <= self.LENGTH_MAX:
                    packet.extend(bytearray([0 for x in range(length+4)]))
                    state = 6
                else:
                    result, rest = -1, data[ii+1:]
                    state = 0
                    break
            elif state == 6:
                # Identifier byte 1
                packet[6] = newByte
                state = 7
            elif state == 7:
                # Identifier byte 2
                packet[7] = newByte
                identifier = unpack('H', packet[6:7+1])[0]
                dataIdx = 0
                state = 8
            elif state == 8:
                # Data bytes
                numBytes = min(len(data[ii:]), len(packet[8+dataIdx:]))
                packet[8+dataIdx:8+dataIdx+numBytes] = data[ii:ii+numBytes]
                dataIdx += numBytes
                if dataIdx >= length+4:
                    # Calculate checksum
                    checksum = self.__crc.Calculate(self.INIT_CHECKSUM, packet[4:]) ^ 0xFFFFFFFF

                    # Test checksum
                    if checksum == 0:
                        self.Identifier = identifier
                        self.Payload = packet[8:-4]
                        result, rest = 1, data[ii+numBytes+1:]
                    else:
                        result, rest = -1, data[ii+numBytes+1:]
                    state = 0
                    break
            else:
                result, rest = -1, data[ii:];
                state = 0

        self.Packet = packet
        self.__Context.Control.State = state
        self.__Context.Control.DataIdx = dataIdx
        self.__Context.Length = length
        self.__Context.Identifier = identifier
        return result, rest
