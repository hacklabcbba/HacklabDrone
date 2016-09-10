#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <zlib.h>
#include "crc32.h"

#define INIT_VALUE (0x00000000)

void debugPrint(void *block, size_t size)
{
	uint8_t *data = (uint8_t*)block;
	for(size_t i=0; i<size; i++)
	{
		printf("0x%02X ", data[i]);
	}
	printf("\r\n");
}

int main(void)
{
	uint8_t dataTest[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
	uint32_t crcTest[4] = {[0 ... 3] = INIT_VALUE};

	printf("InitCRC: ");
	debugPrint(&crcTest[0], 4);
	printf("\r\n");

	for(size_t i=0; i<sizeof(dataTest); i++)
	{
		crcTest[0] = crc32(crcTest[0], &dataTest[i], 1);
		crcTest[1] = CRC32_Bitwise(crcTest[1], &dataTest[i], 1);
		crcTest[2] = CRC32_Lut16(crcTest[2], &dataTest[i], 1);
		crcTest[3] = CRC32_Lut256(crcTest[3], &dataTest[i], 1);

		printf("Data: ");
		debugPrint(&dataTest[i], 1);
		printf("CRC1: ");
		debugPrint(&crcTest[0], 4);
		printf("CRC2: ");
		debugPrint(&crcTest[1], 4);
		printf("CRC3: ");
		debugPrint(&crcTest[2], 4);
		printf("CRC4: ");
		debugPrint(&crcTest[3], 4);
		printf("\r\n");
	}

	crcTest[0] = ~crc32(~crcTest[0], (uint8_t*)&crcTest[0], 4);
	crcTest[1] = ~CRC32_Bitwise(~crcTest[1], (uint8_t*)&crcTest[1], 4);
	crcTest[2] = ~CRC32_Lut16(~crcTest[2], (uint8_t*)&crcTest[2], 4);
	crcTest[3] = ~CRC32_Lut256(~crcTest[3], (uint8_t*)&crcTest[3], 4);

	printf("Verif\r\n");
	printf("CRC1: ");
	debugPrint(&crcTest[0], 4);
	printf("CRC2: ");
	debugPrint(&crcTest[1], 4);
	printf("CRC3: ");
	debugPrint(&crcTest[2], 4);
	printf("CRC4: ");
	debugPrint(&crcTest[3], 4);

	return 0;
}
