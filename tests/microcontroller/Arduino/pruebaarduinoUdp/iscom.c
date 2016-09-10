#include "iscom.h"
#include <string.h>
#include <stdlib.h>

/**********************************************************************************************/

void ISCOM_DeInit(iscom_message_t *Message)
{
	memset(Message, 0, ISCOM_MESSAGE_SIZE);
}

inline void ISCOM_SetContent(iscom_message_t *Message, void *Content, size_t ContentSize)
{
	Message->Header.Size = ContentSize;
	Message->Content = (ContentSize > 0) ? Content : NULL;
}

inline void ISCOM_SetIdentifier(iscom_message_t *Message, uint8_t Primary, uint8_t Secundary)
{
	Message->Header.Identifier.Primary = Primary;
	Message->Header.Identifier.Secundary = Secundary;
}

inline void *ISCOM_GetContent(iscom_message_t *Message)
{
	return Message->Content;
}

inline void ISCOM_GetIdentifier(iscom_message_t *Message, uint8_t *Primary, uint8_t *Secundary)
{
	*Primary = Message->Header.Identifier.Primary;
	*Secundary = Message->Header.Identifier.Secundary;
}

inline size_t ISCOM_GetContentSize(iscom_message_t *Message)
{
	return Message->Header.Size;
}

inline size_t ISCOM_GetPackedSize(iscom_message_t *Message)
{
	size_t MessageSize, ContentSize;
	ContentSize = Message->Header.Size;
	MessageSize = ContentSize + ISCOM_HEADER_SIZE;
	return MessageSize;
}

size_t ISCOM_Pack(iscom_message_t *Message, void *Buffer)
{
	size_t MessageSize, ContentSize;
	ContentSize = Message->Header.Size;
	MessageSize = ContentSize + ISCOM_HEADER_SIZE;
	Message->Header.Size = MessageSize;
	memcpy(Buffer, &Message->Header, ISCOM_HEADER_SIZE);
	if(ContentSize > 0)
	{
		if(Message->Content != NULL)
		{
			memcpy((uint8_t *)Buffer + ISCOM_HEADER_SIZE, Message->Content, ContentSize);
		}
		else
		{
			memset((uint8_t *)Buffer + ISCOM_HEADER_SIZE, 0, ContentSize);
		}
	}
	return MessageSize;
}

size_t ISCOM_UnPack(iscom_message_t *Message, void *Buffer)
{
	size_t MessageSize, ContentSize;
	void **pContent = (void **)&Message->Content;
	uint16_t *pSize = (uint16_t *)&Message->Header.Size;
	memcpy(&Message->Header, Buffer, ISCOM_HEADER_SIZE);
	MessageSize = Message->Header.Size;
	ContentSize = MessageSize - ISCOM_HEADER_SIZE;
	*pSize = ContentSize;
	*pContent = (ContentSize > 0) ? (uint8_t *)Buffer + ISCOM_HEADER_SIZE : NULL;
	return MessageSize;
}