
def crc32(crc, data):
    crc = crc ^ (data << 0);
    for i in range(32):
        if (crc & 0x80000000):
            crc = (crc << 1) ^ 0x04C11DB7
        else:
            crc = (crc << 1)
        crc = crc & 0xFFFFFFFF
    return crc

def crc8(crc, data):
    crc = crc ^ (data << 24);
    for i in range(8):
        if (crc & 0x80000000):
            crc = (crc << 1) ^ 0x04C11DB7
        else:
            crc = (crc << 1)
        crc = crc & 0xFFFFFFFF
    return crc


crc_init = 0xffffffff
data = [ 0 for x in xrange(10)]

data[0] = 0x08;
data[1] = 0x00;

data[2] = 0x0a;
data[3] = 0x0a;
data[4] = 0x0a;
data[5] = 0x0a;

data[6] = 0x0b;
data[7] = 0x0b;
data[8] = 0x0b;
data[9] = 0x0b;

for value in data:
    Crc = crc8(crc_init, value)
    crc_init = Crc
    print hex(Crc & 0xFFFFFFFF)
