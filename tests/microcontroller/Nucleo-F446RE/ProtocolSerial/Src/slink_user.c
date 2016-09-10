#include "slink.h"
#include "stm32f4xx.h"
#include "crc32.h"
#include <string.h>

static uint32_t ReverseCRC32(uint32_t currentCRC, uint32_t desiredCRC)
{
	static const uint32_t CrcTable[16] = {
			0x00000000, 0xB2B4BCB6, 0x61A864DB, 0xD31CD86D,
			0xC350C9B6, 0x71E47500, 0xA2F8AD6D, 0x104C11DB,
			0x82608EDB, 0x30D4326D, 0xE3C8EA00, 0x517C56B6,
			0x4130476D, 0xF384FBDB, 0x209823B6, 0x922C9F00,
	}; // For 0x04C11DB7 Reverse Polynomial
	desiredCRC = (desiredCRC >> 4) ^ CrcTable[desiredCRC & 0x0F];
	desiredCRC = (desiredCRC >> 4) ^ CrcTable[desiredCRC & 0x0F];
	desiredCRC = (desiredCRC >> 4) ^ CrcTable[desiredCRC & 0x0F];
	desiredCRC = (desiredCRC >> 4) ^ CrcTable[desiredCRC & 0x0F];
	desiredCRC = (desiredCRC >> 4) ^ CrcTable[desiredCRC & 0x0F];
	desiredCRC = (desiredCRC >> 4) ^ CrcTable[desiredCRC & 0x0F];
	desiredCRC = (desiredCRC >> 4) ^ CrcTable[desiredCRC & 0x0F];
	desiredCRC = (desiredCRC >> 4) ^ CrcTable[desiredCRC & 0x0F];
	return (desiredCRC ^ currentCRC);
}

void SLink_CalculateChecksum(uint32_t *Checksum, void *Block, size_t Size)
{
	uint32_t *Data = (uint32_t *)Block;
	if(*Checksum == 0)
	{
		WRITE_REG(CRC->CR, CRC_CR_RESET);
	}
	else
	{
		uint32_t desiredChecksum = ~__RBIT(*Checksum);
		uint32_t currentChecksum = CRC->DR;
		if(currentChecksum != desiredChecksum)
		{
			CRC->DR = ReverseCRC32(currentChecksum, desiredChecksum);
		}
	}
	size_t Size32 = Size >> 2;
	while(Size32--)
	{
		CRC->DR = __RBIT(*Data++);
	}
	*Checksum = CRC32_Lut256(~__RBIT(CRC->DR), (uint8_t *)Data, Size & 0x03);
}