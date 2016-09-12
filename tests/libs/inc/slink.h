#ifndef __SLINK_H
#define __SLINK_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**********************************************************************************************/

#define SLINK_SOP_BYTE1        (0x73)
#define SLINK_SOP_BYTE2        (0x6c)
#define SLINK_CHECKSUM_INIT    (0x00000000U)
#define SLINK_MAX_PAYLOAD_SIZE (1000)

/**********************************************************************************************/

typedef struct _slink_message_t *slink_message_t;

typedef struct
{
	size_t PacketCount;
	size_t PacketError;
	size_t PacketRate;
} slink_statistics_t;

typedef enum
{
	SLINK_ERROR = -1,
	SLINK_NULL = 0,
	SLINK_OK = 1,
} slink_returnCodes_t;

/**********************************************************************************************/

slink_message_t SLink_Create(size_t Size);
void SLink_Destroy(slink_message_t Message);

void SLink_InitMessage(slink_message_t Message);
int32_t SLink_EndMessage(slink_message_t Message);

/**********************************************************************************************/

int16_t SLink_GetIdentifier(slink_message_t Message);
void SLink_SetIdentifier(slink_message_t Message, uint16_t Identifier);

void *SLink_GetPayload(slink_message_t Message, size_t *size);
void *SLink_GetPacket(slink_message_t Message, size_t *size);

size_t SLink_GetPayloadSpaceLeft(slink_message_t Message);

/**********************************************************************************************/

int32_t SLink_LoadPayload(slink_message_t Message, void *Payload, size_t Size);
int32_t SLink_ReceiveMessage(slink_message_t Message, uint8_t NewByte);
int32_t SLink_ReceiveMessageBlock(slink_message_t Message, uint8_t *Data, size_t Size, size_t *Idx);

/**********************************************************************************************/

void *SLink_Malloc(size_t Size);
void SLink_Free(void *Memory);
void SLink_CalculateChecksum(uint32_t *Checksum, void *Block, size_t Size);

/**********************************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SLINK_H */