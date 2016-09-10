#include "slink.h"
#include "stm32f7xx.h"
#include "crc32.h"
#include <string.h>

void SLink_CalculateChecksumH(uint32_t *Checksum, uint8_t Data)
{
	SET_BIT(CRC->CR, CRC_CR_RESET);
	MODIFY_REG(CRC->CR, CRC_CR_REV_IN, CRC_CR_REV_IN_0);
	MODIFY_REG(CRC->CR, CRC_CR_REV_OUT, CRC_CR_REV_OUT);

	uint32_t desiredChecksum = ~__RBIT(*Checksum);
	WRITE_REG(CRC->INIT, desiredChecksum);
	*(uint8_t *)(&CRC->DR) = Data;
	*Checksum = ~CRC->DR;
}

void SLink_CalculateBlockChecksum(uint32_t *Checksum, void *Block, size_t Size)
{
	uint32_t *Data = (uint32_t *)Block;
	SET_BIT(CRC->CR, CRC_CR_RESET);
	MODIFY_REG(CRC->CR, CRC_CR_REV_IN, CRC_CR_REV_IN);
	MODIFY_REG(CRC->CR, CRC_CR_REV_OUT, CRC_CR_REV_OUT);

	uint32_t desiredChecksum = ~__RBIT(*Checksum);
	WRITE_REG(CRC->INIT, desiredChecksum);
	size_t Size32 = Size >> 2;
	while(Size32--)
	{
		CRC->DR = *Data++;
	}
	*Checksum = CRC32_Lut16(~CRC->DR, (uint8_t *)Data, Size & 0x03);
}