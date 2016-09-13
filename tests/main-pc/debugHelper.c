#include "debugHelper.h"
#include "stdio.h"

void DEBUG_RawMessagePrint(void *buffer, size_t size)
{
	for(uint32_t i=0; i<size; i++)
	{
		printf(" 0x%02X", ((uint8_t*) buffer)[i]);
	}
	printf("\r\n");
}

void DEBUG_SlinkMessagePrint(slink_message_t message)
{
	size_t payloadSize;
	SLink_GetPayload(message, &payloadSize);
	uint16_t identifier = SLink_GetIdentifier(message);
	printf("Identifier: 0x%04X Size: %zu", identifier, payloadSize);
	printf("\r\n");
}

void DEBUG_IscomPackedMessagePrint(void *buffer, size_t size)
{
	size_t sum = 0;
	while(sum < size)
	{
		iscom_message_t message;
		sum += ISCOM_UnPack(&message, buffer + sum);

		uint8_t idMajor, idMinor;
		ISCOM_GetIdentifier(&message, &idMajor, &idMinor);
		size_t contentSize = ISCOM_GetContentSize(&message);

		printf("Size=%zu IdMajor=0x%02X IdMinor=0x%02X", contentSize, idMajor, idMinor);

		timestamp_t *time = ISCOM_GetContent(&message);
		printf(" Time=%u:%u:%u", time->seconds, time->fraction/1000, time->fraction%1000);
		if(idMajor == 0x01)
		{
			if(idMinor == 0x1A)
			{
				SensorGyro_t *sensor = ISCOM_GetContent(&message);
				printf(" Data=%u, %u, %u", sensor->data[0].elem[0], sensor->data[0].elem[1], sensor->data[0].elem[2]);
			}
			if(idMinor == 0x1B)
			{
				SensorAccel_t *sensor = ISCOM_GetContent(&message);
				printf(" Data=%u, %u, %u", sensor->data[0].elem[0], sensor->data[0].elem[1], sensor->data[0].elem[2]);
			}
			if(idMinor == 0x1C)
			{
				SensorMagnet_t *sensor = ISCOM_GetContent(&message);
				printf(" Data=%u, %u, %u", sensor->data[0].elem[0], sensor->data[0].elem[1], sensor->data[0].elem[2]);
			}
		}
		else if(idMajor == 0x02)
		{
			if(idMinor == 0x1A)
			{
				Control_t *control = ISCOM_GetContent(&message);
				printf(" Data=%u, %u, %u, %u", control->data[0].elem[0], control->data[0].elem[1], control->data[0].elem[2], control->data[0].elem[3]);
			}
		}
		printf("\r\n");
	}
}

