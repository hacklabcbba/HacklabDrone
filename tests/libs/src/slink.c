#include "slink.h"
#include "crc32.h"
#include <stdlib.h>
#include <string.h>

/**********************************************************************************************/

#define SLINK_SOP_SIZE         (2)
#define SLINK_RESERVED_SIZE    (2)
#define SLINK_PREAMBLE_SIZE    (sizeof(slink_preamble_t))
#define SLINK_HEADER_SIZE      (sizeof(slink_header_t))
#define SLINK_CHECKSUM_SIZE    (sizeof(uint32_t))
#define SLINK_PACKET_MIN_SIZE  (sizeof(slink_packet_t))
#define SLINK_MESSAGE_MIN_SIZE (sizeof(struct _slink_message_t))

/**********************************************************************************************/

typedef enum
{
	PREAM_BYTE1_IDX = 0x00,
	PREAM_BYTE2_IDX,
	PREAM_BYTE3_IDX,
	PREAM_BYTE4_IDX,
	HEAD_BYTE1_IDX,
	HEAD_BYTE2_IDX,
	HEAD_BYTE3_IDX,
	HEAD_BYTE4_IDX,
} slink_packetByteIdx_t;

typedef enum
{
	STATE_INIT = 0x00,
	STATE_PREAM_BYTE1 = 0x00,
	STATE_PREAM_BYTE2,
	STATE_PREAM_BYTE3,
	STATE_PREAM_BYTE4,
	STATE_HEAD_BYTE1,
	STATE_HEAD_BYTE2,
	STATE_HEAD_BYTE3,
	STATE_HEAD_BYTE4,
	STATE_DATA_BYTES,
} slink_states_t;

/**********************************************************************************************/

typedef struct
{
	uint8_t Sop[SLINK_SOP_SIZE];
	uint8_t Reserved[SLINK_RESERVED_SIZE];
} slink_preamble_t;

typedef struct
{
	uint16_t Length;
	uint16_t Identifier;
} slink_header_t;

typedef struct
{
	slink_preamble_t Preamble;
	slink_header_t Header;
	uint8_t Data[SLINK_CHECKSUM_SIZE];
} slink_packet_t;

/**********************************************************************************************/

typedef struct
{
	size_t PayloadSizeMax;
	union
	{
		slink_packet_t Packet;
		uint8_t PacketRaw[SLINK_PACKET_MIN_SIZE];
	};
} slink_packetUnit_t;

struct _slink_message_t
{
	struct
	{
		struct
		{
			slink_states_t State;
			size_t DataIdx;
		} Control;

		uint32_t Identifier;
		size_t PayloadSize;
		size_t PacketSize;
	} Context;
	slink_packetUnit_t PacketUnit;
};

/**********************************************************************************************/

slink_message_t SLink_Create(size_t Size)
{
	if(Size <= SLINK_MAX_PAYLOAD_SIZE)
	{
		slink_message_t Message = SLink_Malloc(SLINK_MESSAGE_MIN_SIZE + Size);
		if(Message == NULL)
			return NULL;

		memset(Message, 0xFF, SLINK_MESSAGE_MIN_SIZE + Size);
		Message->Context.Control.State = STATE_INIT;
		Message->PacketUnit.PayloadSizeMax = Size;
		return Message;
	}
	else
	{
		return NULL;
	}
}

void SLink_Destroy(slink_message_t Message)
{
	if(Message != NULL)
	{
		SLink_Free(Message);
	}
}

void SLink_InitMessage(slink_message_t Message)
{
	Message->Context.Control.State = STATE_PREAM_BYTE1;
	Message->Context.PacketSize = 0;
	Message->Context.PayloadSize = 0;
	Message->Context.Identifier = 0;
}

int32_t SLink_EndMessage(slink_message_t Message)
{
	Message->Context.Control.State = STATE_INIT;
	Message->Context.PacketSize = SLINK_PACKET_MIN_SIZE + Message->Context.PayloadSize;

	if(Message->Context.PayloadSize <= Message->PacketUnit.PayloadSizeMax)
	{
		Message->PacketUnit.Packet.Preamble.Sop[0] = SLINK_SOP_BYTE1;
		Message->PacketUnit.Packet.Preamble.Sop[1] = SLINK_SOP_BYTE2;
		Message->PacketUnit.Packet.Preamble.Reserved[0] = 0xFF;
		Message->PacketUnit.Packet.Preamble.Reserved[1] = 0xFF;
		Message->PacketUnit.Packet.Header.Length = Message->Context.PayloadSize;
		Message->PacketUnit.Packet.Header.Identifier = Message->Context.Identifier;

		uint8_t *pChecksum;
		pChecksum = (uint8_t *)&Message->PacketUnit.Packet.Data[Message->Context.PayloadSize];
		uint32_t Checksum = SLINK_CHECKSUM_INIT;
		SLink_CalculateChecksum(&Checksum, &Message->PacketUnit.Packet.Header, SLINK_HEADER_SIZE + Message->Context.PayloadSize);
		Checksum = ~Checksum;
		memcpy(pChecksum, &Checksum, SLINK_CHECKSUM_SIZE);
		return SLINK_OK;
	}
	return SLINK_ERROR;
}

/**********************************************************************************************/

inline int16_t SLink_GetIdentifier(slink_message_t Message)
{
	return Message->Context.Identifier;
}

inline void *SLink_GetPayload(slink_message_t Message)
{
	return (void *)Message->PacketUnit.Packet.Data;
}

inline size_t SLink_GetPayloadSize(slink_message_t Message)
{
	return Message->Context.PayloadSize;
}

inline size_t SLink_GetPayloadSizeMax(slink_message_t Message)
{
	return Message->PacketUnit.PayloadSizeMax;
}

inline void *SLink_GetPacket(slink_message_t Message)
{
	return (void *)Message->PacketUnit.PacketRaw;
}

inline size_t SLink_GetPacketSize(slink_message_t Message)
{
	return Message->Context.PacketSize;
}

inline void SLink_SetIdentifier(slink_message_t Message, uint16_t Identifier)
{
	Message->Context.Identifier = Identifier;
}

/**********************************************************************************************/

int32_t SLink_LoadPayload(slink_message_t Message, void *Payload, size_t Size)
{
	size_t NewSize = Message->Context.PayloadSize + Size;
	if(NewSize <= Message->PacketUnit.PayloadSizeMax)
	{
		memcpy(&Message->PacketUnit.Packet.Data[Message->Context.PayloadSize], (uint8_t *)Payload, Size);
		Message->Context.PayloadSize = NewSize;
		return SLINK_OK;
	}
	return SLINK_ERROR;
}

int32_t SLink_ReceiveMessage(slink_message_t Message, uint8_t NewByte)
{
	int32_t Result = SLINK_NULL;
	switch(Message->Context.Control.State)
	{
	case STATE_PREAM_BYTE1:
	{
		/* Sync byte 1 */
		if(NewByte == SLINK_SOP_BYTE1)
		{
			Message->PacketUnit.PacketRaw[PREAM_BYTE1_IDX] = NewByte;
			Message->Context.Control.State = STATE_PREAM_BYTE2;
		}
	}
	break;
	case STATE_PREAM_BYTE2:
	{
		/* Sync byte 2 */
		if(NewByte == SLINK_SOP_BYTE2)
		{
			Message->PacketUnit.PacketRaw[PREAM_BYTE2_IDX] = NewByte;
			Message->Context.Control.State = STATE_PREAM_BYTE3;
		}
		else
		{
			Message->Context.Control.State = STATE_PREAM_BYTE1;
			Result = SLINK_ERROR;
		}
	}
	break;
	case STATE_PREAM_BYTE3:
	{
		/* Reserved byte 1 */
		Message->PacketUnit.PacketRaw[PREAM_BYTE3_IDX] = NewByte;
		Message->Context.Control.State = STATE_PREAM_BYTE4;
	}
	break;
	case STATE_PREAM_BYTE4:
	{
		/* Reserved byte 2 */
		Message->PacketUnit.PacketRaw[PREAM_BYTE4_IDX] = NewByte;
		Message->Context.Control.State = STATE_HEAD_BYTE1;
	}
	break;
	case STATE_HEAD_BYTE1:
	{
		/* Size byte 1 */
		Message->PacketUnit.PacketRaw[HEAD_BYTE1_IDX] = NewByte;
		Message->Context.Control.State = STATE_HEAD_BYTE2;
	}
	break;
	case STATE_HEAD_BYTE2:
	{
		/* Size byte 2 */
		Message->PacketUnit.PacketRaw[HEAD_BYTE2_IDX] = NewByte;
		Message->Context.PayloadSize = Message->PacketUnit.Packet.Header.Length;
		if(Message->Context.PayloadSize <= Message->PacketUnit.PayloadSizeMax)
		{
			Message->Context.Control.State = STATE_HEAD_BYTE3;
		}
		else
		{
			Message->Context.Control.State = STATE_PREAM_BYTE1;
			Result = SLINK_ERROR;
		}
	}
	break;
	case STATE_HEAD_BYTE3:
	{
		/* Identifier byte 1 */
		Message->PacketUnit.PacketRaw[HEAD_BYTE3_IDX] = NewByte;
		Message->Context.Control.State = STATE_HEAD_BYTE4;
	}
	break;
	case STATE_HEAD_BYTE4:
	{
		/* Identifier byte 2 */
		Message->PacketUnit.PacketRaw[HEAD_BYTE4_IDX] = NewByte;
		Message->Context.Identifier = Message->PacketUnit.Packet.Header.Identifier;
		Message->Context.Control.DataIdx = 0;
		Message->Context.Control.State = STATE_DATA_BYTES;
	}
	break;
	case STATE_DATA_BYTES:
	{
		/* Data bytes*/
		Message->PacketUnit.Packet.Data[Message->Context.Control.DataIdx] = NewByte;
		Message->Context.Control.DataIdx++;
		if(Message->Context.Control.DataIdx >= Message->Context.PayloadSize + SLINK_CHECKSUM_SIZE)
		{
			/* Calculate checksum */
			uint32_t Checksum = SLINK_CHECKSUM_INIT;
			SLink_CalculateChecksum(&Checksum, &Message->PacketUnit.Packet.Header, Message->Context.PayloadSize + SLINK_HEADER_SIZE + SLINK_CHECKSUM_SIZE);

			/* Test checksum */
			if(~Checksum == 0x00)
			{
				Message->Context.PacketSize = SLINK_PACKET_MIN_SIZE + Message->Context.PayloadSize;
				Result = SLINK_OK;
			}
			else
			{
				Result = SLINK_ERROR;
			}
			Message->Context.Control.State = STATE_PREAM_BYTE1;
		}
	}
	break;
	default:
		Message->Context.Control.State = STATE_PREAM_BYTE1;
		Result = SLINK_ERROR;
	}
	return Result;
}

int32_t SLink_ReceiveMessageBlock(slink_message_t Message, uint8_t *Data, size_t Size, size_t *Count)
{
	size_t Idx;
	int32_t Result = SLINK_NULL;
	for(Idx = *Count; (Idx < Size && Result == SLINK_NULL); Idx++)
	{
		uint8_t NewByte = Data[Idx];
		switch(Message->Context.Control.State)
		{
		case STATE_PREAM_BYTE1:
		{
			/* Sync byte 1 */
			if(NewByte == SLINK_SOP_BYTE1)
			{
				Message->PacketUnit.PacketRaw[PREAM_BYTE1_IDX] = NewByte;
				Message->Context.Control.State = STATE_PREAM_BYTE2;
			}
		}
		break;
		case STATE_PREAM_BYTE2:
		{
			/* Sync byte 2 */
			if(NewByte == SLINK_SOP_BYTE2)
			{
				Message->PacketUnit.PacketRaw[PREAM_BYTE2_IDX] = NewByte;
				Message->Context.Control.State = STATE_PREAM_BYTE3;
			}
			else
			{
				Message->Context.Control.State = STATE_PREAM_BYTE1;
				Result = SLINK_ERROR;
			}
		}
		break;
		case STATE_PREAM_BYTE3:
		{
			/* Reserved byte 1 */
			Message->PacketUnit.PacketRaw[PREAM_BYTE3_IDX] = NewByte;
			Message->Context.Control.State = STATE_PREAM_BYTE4;
		}
		break;
		case STATE_PREAM_BYTE4:
		{
			/* Reserved byte 2 */
			Message->PacketUnit.PacketRaw[PREAM_BYTE4_IDX] = NewByte;
			Message->Context.Control.State = STATE_HEAD_BYTE1;
		}
		break;
		case STATE_HEAD_BYTE1:
		{
			/* Size byte 1 */
			Message->PacketUnit.PacketRaw[HEAD_BYTE1_IDX] = NewByte;
			Message->Context.Control.State = STATE_HEAD_BYTE2;
		}
		break;
		case STATE_HEAD_BYTE2:
		{
			/* Size byte 2 */
			Message->PacketUnit.PacketRaw[HEAD_BYTE2_IDX] = NewByte;
			Message->Context.PayloadSize = Message->PacketUnit.Packet.Header.Length;
			if(Message->Context.PayloadSize <= Message->PacketUnit.PayloadSizeMax)
			{
				Message->Context.Control.State = STATE_HEAD_BYTE3;
			}
			else
			{
				Message->Context.Control.State = STATE_PREAM_BYTE1;
				Result = SLINK_ERROR;
			}
		}
		break;
		case STATE_HEAD_BYTE3:
		{
			/* Identifier byte 1 */
			Message->PacketUnit.PacketRaw[HEAD_BYTE3_IDX] = NewByte;
			Message->Context.Control.State = STATE_HEAD_BYTE4;
		}
		break;
		case STATE_HEAD_BYTE4:
		{
			/* Identifier byte 2 */
			Message->PacketUnit.PacketRaw[HEAD_BYTE4_IDX] = NewByte;
			Message->Context.Identifier = Message->PacketUnit.Packet.Header.Identifier;
			Message->Context.Control.DataIdx = 0;
			Message->Context.Control.State = STATE_DATA_BYTES;
		}
		break;
		case STATE_DATA_BYTES:
		{
			/* Data bytes*/
			size_t DataBytesLeft = Size - Idx;
			size_t PacketBytesLeft = (Message->Context.PayloadSize + SLINK_CHECKSUM_SIZE) - Message->Context.Control.DataIdx;
			size_t NumBytes = (DataBytesLeft <= PacketBytesLeft) ? DataBytesLeft : PacketBytesLeft;
			memcpy(&Message->PacketUnit.Packet.Data[Message->Context.Control.DataIdx], &Data[Idx], NumBytes);
			Message->Context.Control.DataIdx += NumBytes;
			Idx += NumBytes - 1;
			if(Message->Context.Control.DataIdx >= Message->Context.PayloadSize + SLINK_CHECKSUM_SIZE)
			{
				/* Calculate checksum */
				uint32_t Checksum = SLINK_CHECKSUM_INIT;
				SLink_CalculateChecksum(&Checksum, &Message->PacketUnit.Packet.Header, Message->Context.PayloadSize + SLINK_HEADER_SIZE + SLINK_CHECKSUM_SIZE);

				/* Test checksum */
				if(~Checksum == 0x00)
				{
					Message->Context.PacketSize = SLINK_PACKET_MIN_SIZE + Message->Context.PayloadSize;
					Result = SLINK_OK;
				}
				else
				{
					Result = SLINK_ERROR;
				}
				Message->Context.Control.State = STATE_PREAM_BYTE1;
			}
		}
		break;
		default:
			Message->Context.Control.State = STATE_PREAM_BYTE1;
			Result = SLINK_ERROR;
		}
	}
	*Count = Idx;
	return Result;
}

/**********************************************************************************************/

__attribute__((weak)) void *SLink_Malloc(size_t Size)
{
	return malloc(Size);
}

__attribute__((weak)) void SLink_Free(void *Memory)
{
	free(Memory);
}

__attribute__((weak)) void SLink_CalculateChecksum(uint32_t *Checksum, void *Block, size_t Size)
{
	uint8_t *Data = (uint8_t *)Block;
	*Checksum = CRC32_Lut16(*Checksum, Data, Size);
}