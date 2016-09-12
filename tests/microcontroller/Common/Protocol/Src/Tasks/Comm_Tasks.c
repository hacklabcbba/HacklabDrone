#include "main.h"

/**********************************************************************************************/

void COMMSendTask(void *param);
void COMMRecvTask(void *param);

size_t COMMSend_TimeMessage(void *buffer);
void COMMRecv_ClassifyMessage(iscom_message_t *Message);

/**********************************************************************************************/

#define COMMRECVTASK_METHOD (10)
#define TXBUFFER_SIZE (SLINK_MAX_PAYLOAD_SIZE)
#define RXBUFFER_SIZE (SLINK_MAX_PAYLOAD_SIZE)
#define TIMEOUT ((uint32_t)1E6)

/**********************************************************************************************/

uint8_t TXbuffer[TXBUFFER_SIZE];
uint8_t RXbuffer[RXBUFFER_SIZE];

Socket *UDPSocket;

SemaphoreHandle_t semap_recvTask;
TaskHandle_t xCOMMSendTask;
TaskHandle_t xCOMMRecvTask;

slink_statistics_t SendStats;
slink_statistics_t RecvStats;

/**********************************************************************************************/

void COMMManagerTask(void *param)
{
	/* Create semaphores */
	semap_recvTask = xSemaphoreCreateBinary();

	/* Test semaphores */
	configASSERT(semap_recvTask != NULL);

	/* Take semaphores */
	xSemaphoreTake(semap_recvTask, 0);

	/* Create tasks */
	xTaskCreate(COMMSendTask, "COMMSendTask", 1000, NULL, configMAX_PRIORITIES - 2, &xCOMMSendTask);
	xTaskCreate(COMMRecvTask, "COMMRecvTask", 1000, NULL, configMAX_PRIORITIES - 3, &xCOMMRecvTask);
	vTaskDelete(NULL);
}

void COMMSendTask(void *param)
{
	traceLabel traceUserEvent1 = xTraceOpenLabel("SendEvent1");
	traceLabel traceUserEvent2 = xTraceOpenLabel("SendEvent2");
	traceLabel traceUserEvent3 = xTraceOpenLabel("SendEvent3");
	traceLabel traceUserEvent4 = xTraceOpenLabel("SendEvent4");

	/* Configure socket */
	error_t error;
	NetInterface *interface = netGetDefaultInterface();
	UDPSocket = socketOpen(SOCKET_TYPE_DGRAM, SOCKET_IP_PROTO_UDP);
	while(!UDPSocket)
		TRACE_FATAL("ERROR: Open Failed\r\n");
	error = socketBindToInterface(UDPSocket, interface);
	while(error)
		TRACE_FATAL("ERROR: Bind to interface\r\n");
	error = socketBind(UDPSocket, &IP_ADDR_ANY, APP_IPV4_HOST_PORT);
	while(error)
		TRACE_FATAL("ERROR: Bind\r\n");
	error = socketSetTimeout(UDPSocket, 100);
	while(error)
		TRACE_FATAL("ERROR: Invalid parameters\r\n");

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		size_t TXbufferLen = 0;
		if(uxQueueMessagesWaiting(queue_sensors) > 0)
		{
			TXbufferLen += COMMSend_TimeMessage(TXbuffer);
		}
		while(uxQueueMessagesWaiting(queue_sensors) > 0)
		{
			iscom_message_t Message;
			BaseType_t errQueue = xQueueReceive(queue_sensors, &Message, (TickType_t)0);
			if((errQueue) && (ISCOM_GetPackedSize(&Message) + TXbufferLen < TXBUFFER_SIZE))
			{
				TXbufferLen += ISCOM_Pack(&Message, TXbuffer + TXbufferLen);
			}
			else
			{
				break;
			}
		}

		if((TXbufferLen > 0) && (linkStatus))
		{
			vTraceUserEvent(traceUserEvent1);

			SLink_InitMessage(SLink_MessageTX);
			SLink_SetIdentifier(SLink_MessageTX, 0xDB);
			int32_t errSLink = SLink_LoadPayload(SLink_MessageTX, TXbuffer, TXbufferLen);
			if(errSLink != SLINK_ERROR)
			{
				vTraceUserEvent(traceUserEvent2);

				SLink_EndMessage(SLink_MessageTX);
				size_t PacketSize;
				uint8_t *pPacket = SLink_GetPacket(SLink_MessageTX, &PacketSize);
				int32_t errComm = socketSendTo(UDPSocket, &remoteAddr, remotePort, pPacket, PacketSize, NULL, 0);
				SendStats.PacketCount++;
			}
			else
			{
				vTraceUserEvent(traceUserEvent3);
				SendStats.PacketError++;
			}
		}
		xSemaphoreGive(semap_recvTask);
		vTaskDelayUntil(&xLastWakeTime, OS_MS_TO_SYSTICKS(1));
	}
}

#if(COMMRECVTASK_METHOD == 10)
//Metodo directo - un solo paquete
void COMMRecvTask(void *param)
{
	traceLabel traceUserEvent1 = xTraceOpenLabel("RecvEvent1");
	traceLabel traceUserEvent2 = xTraceOpenLabel("RecvEvent2");

	uint64_t Timestamp;
	while(1)
	{
		xSemaphoreTake(semap_recvTask, portMAX_DELAY);

		size_t RXbufferLen;
		BaseType_t errComm = socketReceiveFrom(UDPSocket, &remoteAddr, &remotePort, RXbuffer, sizeof(RXbuffer), &RXbufferLen, SOCKET_FLAG_DONT_WAIT);
		if(!errComm)
		{
			vTraceUserEvent(traceUserEvent1);

			size_t RXbufferIdx = 0;
			while(RXbufferIdx < RXbufferLen)
			{
				int32_t errSLink = SLink_ReceiveMessageBlock(SLink_MessageRX, RXbuffer, RXbufferLen, &RXbufferIdx);
				if(errSLink == SLINK_OK)
				{
					vTraceUserEvent(traceUserEvent2);

					size_t PayloadSize;
					uint8_t *pPayload = SLink_GetPayload(SLink_MessageRX, &PayloadSize);
					uint16_t Identifier = SLink_GetIdentifier(SLink_MessageRX);
					if(Identifier == 0xDB)
					{
						size_t sum = 0;
						while(sum < PayloadSize)
						{
							/* Unpack message from buffer */
							iscom_message_t Message;
							sum += ISCOM_UnPack(&Message, pPayload + sum);
							COMMRecv_ClassifyMessage(&Message);
						}
					}
					Timestamp = GTIME_GetCounter();
					RecvStats.PacketCount++;
				}
				else if(errSLink == SLINK_ERROR)
				{
					RecvStats.PacketError++;
				}
			}
		}
		linkStatus = (GTIME_GetCounter() - Timestamp < TIMEOUT) ? 1 : 0;
		xSemaphoreGive(semap_ctrl);
	}
}

#elif(COMMRECVTASK_METHOD == 11)
//Metodo directo - un solo paquete
void COMMRecvTask(void *param)
{
	traceLabel traceUserEvent1 = xTraceOpenLabel("RecvEvent1");
	traceLabel traceUserEvent2 = xTraceOpenLabel("RecvEvent2");

	uint64_t Timestamp;
	while(1)
	{
		xSemaphoreTake(semap_recvTask, portMAX_DELAY);

		size_t RXbufferLen;
		BaseType_t errComm = socketReceiveFrom(UDPSocket, &remoteAddr, &remotePort, RXbuffer, sizeof(RXbuffer), &RXbufferLen, SOCKET_FLAG_DONT_WAIT);
		if(!errComm)
		{
			vTraceUserEvent(traceUserEvent1);

			for(size_t i = 0; i < RXbufferLen; i++)
			{
				int32_t errSLink = SLink_ReceiveMessage(SLink_MessageRX, RXbuffer[i]);
				if(errSLink == SLINK_OK)
				{
					vTraceUserEvent(traceUserEvent2);

					uint8_t *pPayload = SLink_GetPayload(SLink_MessageRX);
					size_t PayloadSize = SLink_GetPayloadSize(SLink_MessageRX);
					uint16_t Identifier = SLink_GetIdentifier(SLink_MessageRX);
					if(Identifier == 0xDB)
					{
						size_t sum = 0;
						while(sum < PayloadSize)
						{
							/* Unpack message from buffer */
							iscom_message_t Message;
							sum += ISCOM_UnPack(&Message, pPayload + sum);
							COMMRecv_ClassifyMessage(&Message);
						}
					}
					Timestamp = GTIME_GetCounter();
					RecvStats.PacketCount++;
				}
				else if(errSLink == SLINK_ERROR)
				{
					RecvStats.PacketError++;
				}
			}
		}
		linkStatus = (GTIME_GetCounter() - Timestamp < TIMEOUT) ? 1 : 0;
		xSemaphoreGive(semap_ctrl);
	}
}

#elif(COMMRECVTASK_METHOD == 20)
//Metodo basado en eventos - un solo paquete
void COMMRecvTask(void *param)
{
	traceLabel traceUserEvent1 = xTraceOpenLabel("RecvEvent1");
	traceLabel traceUserEvent2 = xTraceOpenLabel("RecvEvent2");

	uint64_t Timestamp;
	OsEvent event;
	osCreateEvent(&event);
	socketRegisterEvents(UDPSocket, &event, SOCKET_EVENT_RX_READY);
	while(1)
	{
		xSemaphoreTake(semap_recvTask, portMAX_DELAY);

		uint_t eventFlags;
		socketGetEvents(UDPSocket, &eventFlags);
		if(eventFlags & SOCKET_EVENT_RX_READY)
		{
			size_t RXbufferLen;
			BaseType_t errComm = socketReceiveFrom(UDPSocket, &remoteAddr, &remotePort, RXbuffer, sizeof(RXbuffer), &RXbufferLen, SOCKET_FLAG_DONT_WAIT);
			if(!errComm)
			{
				vTraceUserEvent(traceUserEvent1);

				for(size_t i = 0; i < RXbufferLen; i++)
				{
					int32_t errSLink = SLink_ReceiveMessage(SLink_MessageRX, RXbuffer[i]);
					if(errSLink == SLINK_OK)
					{
						vTraceUserEvent(traceUserEvent2);

						uint8_t *pPayload = SLink_GetPayload(SLink_MessageRX);
						size_t PayloadSize = SLink_GetPayloadSize(SLink_MessageRX);
						uint16_t Identifier = SLink_GetIdentifier(SLink_MessageRX);
						if(Identifier == 0xDB)
						{
							size_t sum = 0;
							while(sum < PayloadSize)
							{
								/* Unpack message from buffer */
								iscom_message_t Message;
								sum += ISCOM_UnPack(&Message, pPayload + sum);
								COMMRecv_ClassifyMessage(&Message);
							}
						}
						Timestamp = GTIME_GetCounter();
						SendStats.PacketCount++;
					}
					else if(errSLink == SLINK_ERROR)
					{
						RecvStats.PacketError++;
					}
				}
			}
		}
		linkStatus = (GTIME_GetCounter() - Timestamp < TIMEOUT) ? 1 : 0;
		xSemaphoreGive(semap_ctrl);
	}
}

#elif(COMMRECVTASK_METHOD == 30)
//Metodo basado en eventos - varios paquetes
void COMMRecvTask(void *param)
{
	traceLabel traceUserEvent1 = xTraceOpenLabel("RecvEvent1");
	traceLabel traceUserEvent2 = xTraceOpenLabel("RecvEvent2");

	uint64_t Timestamp;
	OsEvent event;
	osCreateEvent(&event);
	socketRegisterEvents(UDPSocket, &event, SOCKET_EVENT_RX_READY);
	while(1)
	{
		xSemaphoreTake(semap_recvTask, portMAX_DELAY);

		uint32_t count = 0;
		uint_t eventFlags;
		socketGetEvents(UDPSocket, &eventFlags);
		while((eventFlags & SOCKET_EVENT_RX_READY) && (count++ < 4))
		{
			size_t RXbufferLen;
			BaseType_t errComm = socketReceiveFrom(UDPSocket, &remoteAddr, &remotePort, RXbuffer, sizeof(RXbuffer), &RXbufferLen, SOCKET_FLAG_DONT_WAIT);
			if(!errComm)
			{
				vTraceUserEvent(traceUserEvent1);

				for(size_t i = 0; i < RXbufferLen; i++)
				{
					int32_t errSLink = SLink_ReceiveMessage(SLink_MessageRX, RXbuffer[i]);
					if(errSLink == SLINK_OK)
					{
						vTraceUserEvent(traceUserEvent2);

						uint8_t *pPayload = SLink_GetPayload(SLink_MessageRX);
						size_t PayloadSize = SLink_GetPayloadSize(SLink_MessageRX);
						uint16_t Identifier = SLink_GetIdentifier(SLink_MessageRX);
						if(Identifier == 0xDB)
						{
							size_t sum = 0;
							while(sum < PayloadSize)
							{
								/* Unpack message from buffer */
								iscom_message_t Message;
								sum += ISCOM_UnPack(&Message, pPayload + sum);
								COMMRecv_ClassifyMessage(&Message);
							}
						}
						Timestamp = GTIME_GetCounter();
						SendStats.PacketCount++;
					}
					else if(errSLink == SLINK_ERROR)
					{
						RecvStats.PacketError++;
					}
				}
			}
			socketGetEvents(UDPSocket, &eventFlags);
		}
		linkStatus = (GTIME_GetCounter() - Timestamp < TIMEOUT) ? 1 : 0;
		xSemaphoreGive(semap_ctrl);
	}
}
#endif

/**********************************************************************************************/

size_t COMMSend_TimeMessage(void *buffer)
{
	timestamp_t timestamp;

	/* Read time */
	uint64_t Timestamp = GTIME_GetCounter();
	timestamp.seconds = Timestamp / (uint32_t)1E6;
	timestamp.fraction = Timestamp % (uint32_t)1E6;

	iscom_message_t Message;
	ISCOM_SetIdentifier(&Message, 0x00, 0x01);
	ISCOM_SetContent(&Message, &timestamp, TIMESTAMP_SIZE);
	return ISCOM_Pack(&Message, buffer);
}

/**********************************************************************************************/

void MessageID0x00(iscom_message_t *Message, uint8_t ID2)
{
	switch(ID2)
	{
	case 0x01:
		if(ISCOM_GetContentSize(Message) == TIMESTAMP_SIZE)
		{
			memcpy(&remoteTime, ISCOM_GetContent(Message), ISCOM_GetContentSize(Message));
		}
		break;
	default:
		break;
	}
}

void MessageID0x01(iscom_message_t *Message, uint8_t ID2)
{
}

void MessageID0x02(iscom_message_t *Message, uint8_t ID2)
{
	switch(ID2)
	{
	case 0x1A:
		if(ISCOM_GetContentSize(Message) == SBuffer_GetSize(circBuffer_Control))
		{
			void *pControl = SBuffer_Next(circBuffer_Control);
			memcpy(pControl, ISCOM_GetContent(Message), ISCOM_GetContentSize(Message));
			if(SBuffer_GetNum(circBuffer_Control) == 1)
			{
				xQueueOverwrite(queue_control, (void *)Message);
			}
			else
			{
				xQueueSend(queue_control, (void *)Message, (TickType_t)0);
			}
		}
		break;
	default:
		break;
	}
}

void COMMRecv_ClassifyMessage(iscom_message_t *Message)
{
	uint8_t ID1, ID2;
	ISCOM_GetIdentifier(Message, &ID1, &ID2);
	switch(ID1)
	{
	case 0x00:
		MessageID0x00(Message, ID2);
		break;
	case 0x01:
		MessageID0x01(Message, ID2);
		break;
	case 0x02:
		MessageID0x02(Message, ID2);
		break;
	}
}

/**********************************************************************************************/