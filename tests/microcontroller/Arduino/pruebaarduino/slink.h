#ifndef __SLINK_H
#define __SLINK_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/**********************************************************************************************/

typedef struct __SLINK_MESSAGE *SLink_Message_t;
extern size_t SLINK_MAX_PAYLOAD_SIZE;

#define SLINK_SOP_BYTE1        (0x73)
#define SLINK_SOP_BYTE2        (0x6c)
#define SLINK_EOP_BYTE         (0xFF)
#define SLINK_CHECKSUM_INIT    (0xFFFFFFFFU)
#define SLINK_MAX_PAYLOAD_SIZE (512)

enum
{
	SLINK_ERROR = -1,
	SLINK_NULL = 0,
	SLINK_OK = 1,
};

/**********************************************************************************************/

SLink_Message_t SLink_Create(size_t Size);
void SLink_Destroy(SLink_Message_t SLinkMessage);

void SLink_InitMessage(SLink_Message_t SLinkMessage);
int32_t SLink_EndMessage(SLink_Message_t SLinkMessage);

/**********************************************************************************************/

int16_t SLink_GetPortID(SLink_Message_t SLinkMessage);
size_t SLink_GetPayloadSize(SLink_Message_t SLinkMessage);
size_t SLink_GetPayloadSizeMax(SLink_Message_t SLinkMessage);
void *SLink_GetPayload(SLink_Message_t SLinkMessage);
void *SLink_GetPacket(SLink_Message_t SLinkMessage);
size_t SLink_GetPacketSize(SLink_Message_t SLinkMessage);

void SLink_SetIdentifier(SLink_Message_t SLinkMessage, uint16_t Identifier);
int32_t SLink_SetPayloadSize(SLink_Message_t SLinkMessage, size_t Size);

/**********************************************************************************************/

int32_t SLink_LoadPayload(SLink_Message_t SLinkMessage, void *Payload, size_t Size);
int32_t SLink_ReceiveMessage(SLink_Message_t SLinkMessage, uint8_t NewByte);

/**********************************************************************************************/

void SLink_CalculateChecksum(uint32_t *Checksum, uint8_t Data);
void SLink_CalculateBlockChecksum(uint32_t *Checksum, void *Block, size_t Size);

/**********************************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SLINK_H */
