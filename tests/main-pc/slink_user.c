#include <stdint.h>
#include <zlib.h>

void SLink_CalculateChecksum(uint32_t *Checksum, void *Block, size_t Size)
{
	uint8_t *Data = (uint8_t *)Block;
	*Checksum = crc32(*Checksum, Data, Size);
}
