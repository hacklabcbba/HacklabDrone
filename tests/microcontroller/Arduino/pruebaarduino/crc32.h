#ifndef __CRC32_H
#define __CRC32_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/**********************************************************************************************/

uint32_t CRC32_Bitwise(uint32_t crc, void *block, size_t length);
uint32_t CRC32_Lut16(uint32_t crc, void *block, size_t length);
uint32_t CRC32_Lut256(uint32_t crc, void *block, size_t length);

/**********************************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SBUFFER_H */