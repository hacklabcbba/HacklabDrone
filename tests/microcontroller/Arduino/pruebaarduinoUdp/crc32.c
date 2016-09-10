#include "crc32.h"

/**********************************************************************************************/

#define POLYNOMIAL (0xEDB88320)
#define S2(i)    S1((i)),     S1((i + 1))
#define S4(i)    S2((i)),     S2((i + 2))
#define S8(i)    S4((i)),     S4((i + 4))
#define S16(i)   S8((i)),     S8((i + 8))
#define S32(i)   S16((i)),    S16((i + 16))
#define S64(i)   S32((i)),    S32((i + 32))
#define S128(i)  S64((i)),    S64((i + 64))
#define S256(i)  S128((i)),   S128((i + 128))

#define LutCalc(x)  (((x >> 1) ^ (x & 1) * POLYNOMIAL))
#define LutCalc2(i) LutCalc(LutCalc((i)))
#define LutCalc4(i) LutCalc2(LutCalc2((i)))
#define LutCalc8(i) LutCalc4(LutCalc4((i)))

/**********************************************************************************************/

uint32_t CRC32_Bitwise(uint32_t crc, void *block, size_t length)
{
	uint8_t *data = (uint8_t *)block;
	crc = ~crc;
	while(length--)
	{
		crc = (crc) ^ ((uint32_t)*data++);
		for(size_t i = 0; i < 8; i++)
		{
			if(crc & 0x01)
				crc = (crc >> 1) ^ POLYNOMIAL;
			else
				crc = (crc >> 1);
		}
	}
	return ~crc;
}

uint32_t CRC32_Lut16(uint32_t crc, void *block, size_t length)
{
#define S1(i) LutCalc8((i * 16))
	static const uint32_t lut[16] = {S16(0)};
#undef S1
	uint8_t *data = (uint8_t *)block;
	crc = ~crc;
	while(length--)
	{
		crc = (crc) ^ ((uint32_t)*data++);
		crc = (crc >> 4) ^ lut[crc & 0x0f];
		crc = (crc >> 4) ^ lut[crc & 0x0f];
	}
	return ~crc;
}

uint32_t CRC32_Lut256(uint32_t crc, void *block, size_t length)
{
#define S1(i) LutCalc8((i))
	static const uint32_t lut[256] = {S256(0)};
#undef S1
	uint8_t *data = (uint8_t *)block;
	crc = ~crc;
	while(length--)
	{
		crc = (crc) ^ ((uint32_t)*data++);
		crc = (crc >> 8) ^ lut[crc & 0xff];
	}
	return ~crc;
}