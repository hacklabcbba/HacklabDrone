#include "slink.h"
#include "crc32.h"
#include <stdlib.h>
#include <string.h>

/**********************************************************************************************/

#define SLINK_SOP_SIZE         (2)
#define SLINK_RESERVED_SIZE    (2)
#define SLINK_PREAMBLE_SIZE    (sizeof(SLink_Preamble_t))
#define SLINK_HEADER_SIZE      (sizeof(SLink_Header_t))
#define SLINK_CHECKSUM_SIZE    (sizeof(uint32_t))
#define SLINK_PACKET_MIN_SIZE  (sizeof(SLink_Packet_t))
#define SLINK_MESSAGE_MIN_SIZE (sizeof(struct __SLINK_MESSAGE))

enum
{
	PREAM_BYTE1_IDX = 0x00,
	PREAM_BYTE2_IDX,
	PREAM_BYTE3_IDX,
	PREAM_BYTE4_IDX,
	HEAD_BYTE1_IDX,
	HEAD_BYTE2_IDX,
	HEAD_BYTE3_IDX,
	HEAD_BYTE4_IDX,
};

typedef enum
{
	STATE_NULL = 0x00,
	STATE_PREAM_BYTE1 = 0x10,
	STATE_PREAM_BYTE2,
	STATE_PREAM_BYTE3,
	STATE_PREAM_BYTE4,
	STATE_HEAD_BYTE1,
	STATE_HEAD_BYTE2,
	STATE_HEAD_BYTE3,
	STATE_HEAD_BYTE4,
	STATE_DATA_BYTES,
} SLink_States_t;

typedef struct
{
	uint8_t Sop[SLINK_SOP_SIZE];
	uint8_t Reserved[SLINK_RESERVED_SIZE];
} SLink_Preamble_t;

typedef struct
{
	uint16_t Length;
	uint16_t Identifier;
} SLink_Header_t;

typedef struct
{
	SLink_Preamble_t Preamble;
	SLink_Header_t Header;
	uint8_t Data[SLINK_CHECKSUM_SIZE];
} SLink_Packet_t;

struct __SLINK_MESSAGE
{
	struct
	{
		uint32_t Identifier;
		size_t PacketSize;
		size_t PayloadSize;
		size_t PayloadSizeMax;
	} Ctrl;

	struct
	{
		SLink_States_t State;
		uint32_t Identifier;
		size_t PayloadSize;
		size_t DataIdx;
		uint32_t Checksum;
	} CtrlPriv;

	union
	{
		SLink_Packet_t Packet;
		uint8_t PacketRaw[SLINK_PACKET_MIN_SIZE];
	};
};

/**********************************************************************************************/

SLink_Message_t SLink_Create(size_t Size)
{
	if(Size <= SLINK_MAX_PAYLOAD_SIZE)
	{
		SLink_Message_t SLinkMessage = malloc(SLINK_MESSAGE_MIN_SIZE + Size);
		if(SLinkMessage == NULL)
			return NULL;

		memset(SLinkMessage, 0xFF, SLINK_MESSAGE_MIN_SIZE + Size);
		SLinkMessage->Ctrl.PayloadSizeMax = Size;
		SLinkMessage->CtrlPriv.State = STATE_NULL;
		return SLinkMessage;
	}
	else
	{
		return NULL;
	}
}

void SLink_Destroy(SLink_Message_t SLinkMessage)
{
	if(SLinkMessage != NULL)
	{
		free(SLinkMessage);
	}
}

void SLink_InitMessage(SLink_Message_t SLinkMessage)
{
	SLinkMessage->CtrlPriv.State = STATE_PREAM_BYTE1;
	SLinkMessage->Ctrl.PacketSize = 0;
	SLinkMessage->Ctrl.PayloadSize = 0;
	SLinkMessage->Ctrl.Identifier = 0;
}

int32_t SLink_EndMessage(SLink_Message_t SLinkMessage)
{
	SLinkMessage->CtrlPriv.State = STATE_NULL;
	SLinkMessage->Ctrl.PacketSize = SLINK_PACKET_MIN_SIZE + SLinkMessage->Ctrl.PayloadSize;
	if(SLinkMessage->Ctrl.PayloadSize <= SLinkMessage->Ctrl.PayloadSizeMax)
	{
		SLinkMessage->Packet.Preamble.Sop[0] = SLINK_SOP_BYTE1;
		SLinkMessage->Packet.Preamble.Sop[1] = SLINK_SOP_BYTE2;
		SLinkMessage->Packet.Preamble.Reserved[0] = 0xFF;
		SLinkMessage->Packet.Preamble.Reserved[1] = 0xFF;
		SLinkMessage->Packet.Header.Length = SLinkMessage->Ctrl.PayloadSize;
		SLinkMessage->Packet.Header.Identifier = SLinkMessage->Ctrl.Identifier;

		uint8_t *pChecksum;
		pChecksum = (uint8_t *)&SLinkMessage->Packet.Data[SLinkMessage->Ctrl.PayloadSize];
		SLinkMessage->CtrlPriv.Checksum = SLINK_CHECKSUM_INIT;
		SLink_CalculateBlockChecksum(&SLinkMessage->CtrlPriv.Checksum, &SLinkMessage->Packet.Header, SLINK_HEADER_SIZE + SLinkMessage->Ctrl.PayloadSize);
		SLinkMessage->CtrlPriv.Checksum = ~SLinkMessage->CtrlPriv.Checksum;
		memcpy(pChecksum, &SLinkMessage->CtrlPriv.Checksum, SLINK_CHECKSUM_SIZE);
		return SLINK_OK;
	}
	return SLINK_ERROR;
}

/**********************************************************************************************/

inline int16_t SLink_GetPortID(SLink_Message_t SLinkMessage)
{
	return SLinkMessage->Ctrl.Identifier;
}

inline size_t SLink_GetPayloadSize(SLink_Message_t SLinkMessage)
{
	return SLinkMessage->Ctrl.PayloadSize;
}

inline size_t SLink_GetPayloadSizeMax(SLink_Message_t SLinkMessage)
{
	return SLinkMessage->Ctrl.PayloadSizeMax;
}

inline void *SLink_GetPayload(SLink_Message_t SLinkMessage)
{
	return (void *)SLinkMessage->Packet.Data;
}

inline void *SLink_GetPacket(SLink_Message_t SLinkMessage)
{
	return (void *)&SLinkMessage->Packet;
}

inline size_t SLink_GetPacketSize(SLink_Message_t SLinkMessage)
{
	return SLinkMessage->Ctrl.PacketSize;
}

inline void SLink_SetIdentifier(SLink_Message_t SLinkMessage, uint16_t Identifier)
{
	SLinkMessage->Ctrl.Identifier = Identifier;
}

inline int32_t SLink_SetPayloadSize(SLink_Message_t SLinkMessage, size_t Size)
{
	if(Size <= SLinkMessage->Ctrl.PayloadSizeMax)
	{
		SLinkMessage->Ctrl.PayloadSize = Size;
		return SLINK_OK;
	}
	return SLINK_ERROR;
}

/**********************************************************************************************/

int32_t SLink_LoadPayload(SLink_Message_t SLinkMessage, void *Payload, size_t Size)
{
	size_t NewSize = SLinkMessage->Ctrl.PayloadSize + Size;
	if(NewSize <= SLinkMessage->Ctrl.PayloadSizeMax)
	{
		memcpy(&SLinkMessage->Packet.Data[SLinkMessage->Ctrl.PayloadSize], (uint8_t *)Payload, Size);
		SLinkMessage->Ctrl.PayloadSize = NewSize;
		return SLinkMessage->Ctrl.PayloadSize;
	}
	return SLINK_ERROR;
}

int32_t SLink_ReceiveMessage(SLink_Message_t SLinkMessage, uint8_t NewByte)
{
	switch(SLinkMessage->CtrlPriv.State)
	{
	case STATE_PREAM_BYTE1:
	{
		/* Sync byte 1 */
		if(NewByte == SLINK_SOP_BYTE1)
		{
			SLinkMessage->PacketRaw[PREAM_BYTE1_IDX] = NewByte;
			SLinkMessage->CtrlPriv.State = STATE_PREAM_BYTE2;
		}
	}
	break;
	case STATE_PREAM_BYTE2:
	{
		/* Sync byte 2 */
		if(NewByte == SLINK_SOP_BYTE2)
		{
			SLinkMessage->PacketRaw[PREAM_BYTE2_IDX] = NewByte;
			SLinkMessage->CtrlPriv.State = STATE_PREAM_BYTE3;
		}
		else
		{
			SLinkMessage->CtrlPriv.State = STATE_PREAM_BYTE1;
			return SLINK_ERROR;
		}
	}
	break;
	case STATE_PREAM_BYTE3:
	{
		/* Reserved byte 1 */
		SLinkMessage->PacketRaw[PREAM_BYTE3_IDX] = NewByte;
		SLinkMessage->CtrlPriv.State = STATE_PREAM_BYTE4;
	}
	break;
	case STATE_PREAM_BYTE4:
	{
		/* Reserved byte 2 */
		SLinkMessage->PacketRaw[PREAM_BYTE4_IDX] = NewByte;
		SLinkMessage->CtrlPriv.State = STATE_HEAD_BYTE1;
	}
	break;
	case STATE_HEAD_BYTE1:
	{
		/* Size byte 1 */
		SLinkMessage->PacketRaw[HEAD_BYTE1_IDX] = NewByte;
		SLinkMessage->CtrlPriv.Checksum = SLINK_CHECKSUM_INIT;
		SLink_CalculateChecksum(&SLinkMessage->CtrlPriv.Checksum, NewByte);
		SLinkMessage->CtrlPriv.State = STATE_HEAD_BYTE2;
	}
	break;
	case STATE_HEAD_BYTE2:
	{
		/* Size byte 2 */
		SLinkMessage->PacketRaw[HEAD_BYTE2_IDX] = NewByte;
		SLinkMessage->CtrlPriv.PayloadSize = SLinkMessage->Packet.Header.Length;
		if(SLinkMessage->CtrlPriv.PayloadSize <= SLinkMessage->Ctrl.PayloadSizeMax)
		{
			SLink_CalculateChecksum(&SLinkMessage->CtrlPriv.Checksum, NewByte);
			SLinkMessage->CtrlPriv.State = STATE_HEAD_BYTE3;
		}
		else
		{
			SLinkMessage->CtrlPriv.State = STATE_PREAM_BYTE1;
			return SLINK_ERROR;
		}
	}
	break;
	case STATE_HEAD_BYTE3:
	{
		/* PortID byte 1 */
		SLinkMessage->PacketRaw[HEAD_BYTE3_IDX] = NewByte;
		SLink_CalculateChecksum(&SLinkMessage->CtrlPriv.Checksum, NewByte);
		SLinkMessage->CtrlPriv.State = STATE_HEAD_BYTE4;
	}
	break;
	case STATE_HEAD_BYTE4:
	{
		/* PortID byte 2 */
		SLinkMessage->PacketRaw[HEAD_BYTE4_IDX] = NewByte;
		SLinkMessage->CtrlPriv.Identifier = SLinkMessage->Packet.Header.Identifier;
		SLinkMessage->CtrlPriv.DataIdx = 0;
		SLink_CalculateChecksum(&SLinkMessage->CtrlPriv.Checksum, NewByte);
		SLinkMessage->CtrlPriv.State = STATE_DATA_BYTES;
	}
	break;
	case STATE_DATA_BYTES:
	{
		/* Data bytes*/
		SLinkMessage->Packet.Data[SLinkMessage->CtrlPriv.DataIdx] = NewByte;
		SLinkMessage->CtrlPriv.DataIdx++;
		SLink_CalculateChecksum(&SLinkMessage->CtrlPriv.Checksum, NewByte);
		if(SLinkMessage->CtrlPriv.DataIdx >= SLinkMessage->CtrlPriv.PayloadSize + SLINK_CHECKSUM_SIZE)
		{
			SLinkMessage->CtrlPriv.State = STATE_PREAM_BYTE1;
			if(~SLinkMessage->CtrlPriv.Checksum == 0x00)
			{
				SLinkMessage->Ctrl.PacketSize = SLINK_PACKET_MIN_SIZE + SLinkMessage->CtrlPriv.PayloadSize;
				SLinkMessage->Ctrl.PayloadSize = SLinkMessage->CtrlPriv.PayloadSize;
				SLinkMessage->Ctrl.Identifier = SLinkMessage->CtrlPriv.Identifier;
				return SLINK_OK;
			}
			else
			{
				return SLINK_ERROR;
			}
		}
	}
	break;
	default:
		SLinkMessage->CtrlPriv.State = STATE_PREAM_BYTE1;
		return SLINK_ERROR;
	}
	return SLINK_NULL;
}

/**********************************************************************************************/

__attribute__((weak)) void SLink_CalculateChecksum(uint32_t *Checksum, uint8_t Data)
{
	*Checksum = CRC32_Lut16(*Checksum, &Data, 1);
}

__attribute__((weak)) void SLink_CalculateBlockChecksum(uint32_t *Checksum, void *Block, size_t Size)
{
	uint8_t *Data = (uint8_t *)Block;
	*Checksum = CRC32_Lut16(*Checksum, Data, Size);
}