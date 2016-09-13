#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "slink.h"

#define CYCLES 100000
#define PAYLOADSIZE 100
#define PADSIZE 4

double timeDiff(struct timespec tStart, struct timespec tEnd)
{
	return (double)(tEnd.tv_sec - tStart.tv_sec) + (double)(tEnd.tv_nsec - tStart.tv_nsec)*1.0e-9;
}

int main(int argc, char *argv[])
{
	uint8_t *payload = calloc(1, PAYLOADSIZE);

	slink_message_t messageTX;
	slink_message_t messageRX;
	messageTX = SLink_Create(SLINK_MAX_PAYLOAD_SIZE);
	messageRX = SLink_Create(SLINK_MAX_PAYLOAD_SIZE);
	size_t count;
	double dt;
	struct timespec t1, t2;

	printf("Cycles: %d PayloadSize: %d PadSize: %d\n", CYCLES, PAYLOADSIZE, PADSIZE);

	count = 0;
	clock_gettime(CLOCK_REALTIME, &t1);
	for(size_t i=0; i<CYCLES; i++)
	{
		SLink_InitMessage(messageTX);
		SLink_SetIdentifier(messageTX, 0xDB);
		SLink_LoadPayload(messageTX, payload, PAYLOADSIZE);
		SLink_EndMessage(messageTX);
		count++;
	}
	clock_gettime(CLOCK_REALTIME, &t2);
	dt = timeDiff(t1, t2)*1.0e3;
	printf("Time: %0.3f Count: %zu\n", dt, count);

	size_t bufferLen;
	uint8_t *tmp = SLink_GetPacket(messageTX, &bufferLen);
	uint8_t *buffer = calloc(1, bufferLen + PADSIZE);
	memcpy(buffer, tmp, bufferLen);
	bufferLen += PADSIZE;

	count = 0;
	clock_gettime(CLOCK_REALTIME, &t1);
	for(size_t i=0; i<CYCLES; i++)
	{
		size_t bufferIdx = 0;
		while(bufferIdx < bufferLen)
		{
			int32_t errSLink  = SLink_ReceiveMessageBlock(messageRX, buffer, bufferLen, &bufferIdx);
			if(errSLink == SLINK_OK)
			{
				count++;
			}
		}
	}
	clock_gettime(CLOCK_REALTIME, &t2);
	dt = timeDiff(t1, t2)*1.0e3;
	printf("Time: %0.3f Count: %zu\n", dt, count);

	free(payload);
	free(buffer);
}

