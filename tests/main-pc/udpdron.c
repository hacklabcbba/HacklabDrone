#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <string.h>

#include <fcntl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <arpa/inet.h>

#include "iscom.h"
#include "sbuffer.h"
#include "slink.h"
#include "transport_types.h"

#include "debugHelper.h"

#define TRACEPOINT_CREATE_PROBES
#define TRACEPOINT_DEFINE
#include "tpoint.h"

/**********************************************************************************************/

void die(char *s);
void print_usage(void);

int Socket_Create(void);
void Socket_Config(int fd, int portNumber);
int Socket_Send(int fd, struct sockaddr_in *remoteAddr, void *data, size_t size);
int Socket_Recv(int fd, struct sockaddr_in *remoteAddr, void *data, size_t size);
int Timer_Create(void);
void Timer_Config(int fd, uint32_t time);
void Pipe_Create(int fd[2]);

void debug_MessageTx_(slink_message_t Message);
void debug_MessageRx_(slink_message_t Message);

/**********************************************************************************************/

#define DEFAULT_REMOTE_ADDR  "169.254.1.1"
#define DEFAULT_PORT         5000
#define DEFAULT_TIMER_PERIOD 1000

#define TXBUFFER_SIZE        1024
#define RXBUFFER_SIZE        1024

#define TIME_BUFFER_SIZE     10
#define SENSOR_BUFFER_SIZE   10
#define CONTROL_BUFFER_SIZE  10

//#define DEBUG_MESSAGE_TX
//#define DEBUG_MESSAGE_RX

sbuffer_t circBuffer_Time;
sbuffer_t circBuffer_Control;
sbuffer_t circBuffer_SensorGyro;
sbuffer_t circBuffer_SensorAccel;
sbuffer_t circBuffer_SensorMagnet;

size_t SendCount = 0;
size_t RecvCount = 0;
slink_statistics_t SendStats;
slink_statistics_t RecvStats;

/**********************************************************************************************/

void ControlSimulate(int pipefd)
{
	static struct timespec timeLast;
	static double count=0;
	static int dir = 1;

	struct timespec time;
	static Control_t Control;
	sbuffer_t ControlBuffer = circBuffer_Control;
	size_t ControlSize = SBuffer_GetSize(ControlBuffer);
	iscom_message_t Message;

	clock_gettime(CLOCK_REALTIME, &time);
	Control.timestamp.seconds = time.tv_sec;
	Control.timestamp.fraction = time.tv_nsec / 1000;
	Control.data[0].elem[0] = 1500;
	Control.data[0].elem[1] = 1000;
	Control.data[0].elem[2] = 2000;
	Control.data[0].elem[3] = count;

	double timeDiff = (double)(time.tv_sec - timeLast.tv_sec) + (double)(time.tv_nsec - timeLast.tv_nsec)*1.0e-9;
	timeLast = time;
	count = count + dir * timeDiff * 1000;
	dir = (count > 2300)? -1 : (count < 700)? 1 : dir;
	count = (count > 2300)? 2300 : (count < 700)? 700 : count;

	ISCOM_DeInit(&Message);
	ISCOM_SetIdentifier(&Message, 0x02, 0x1A);
	void *pControl = SBuffer_Next(ControlBuffer);
	memcpy(pControl, &Control, ControlSize);
	ISCOM_SetContent(&Message, pControl, ControlSize);
	write(pipefd, &Message, ISCOM_MESSAGE_SIZE);
}

size_t COMMSend_TimeMessage(void *buffer)
{
	timestamp_t timestamp;

	/* Read time */
	struct timespec tms;
	clock_gettime(CLOCK_REALTIME, &tms);
	timestamp.seconds = tms.tv_sec;
	timestamp.fraction = tms.tv_nsec / 1000;

	iscom_message_t Message;
	ISCOM_SetIdentifier(&Message, 0x00, 0x01);
	ISCOM_SetContent(&Message, &timestamp, TIMESTAMP_SIZE);
	return ISCOM_Pack(&Message, buffer);
}

/**********************************************************************************************/

void COMMSendTask(slink_message_t SLink_MessageTX, int pipefd, int socketfd, struct sockaddr_in *remoteAddr)
{
	uint8_t TXbuffer[TXBUFFER_SIZE];
	size_t TXbufferLen = 0;

	TXbufferLen += COMMSend_TimeMessage(TXbuffer);
	iscom_message_t Message;
	while (read(pipefd, &Message, ISCOM_MESSAGE_SIZE) > 0)
	{
		if(ISCOM_GetPackedSize(&Message) + TXbufferLen < TXBUFFER_SIZE)
		{
			TXbufferLen += ISCOM_Pack(&Message, TXbuffer + TXbufferLen);
		}
		else
		{
			break;
		}
	}

	if(TXbufferLen > 0)
	{
		/* Create slink message */
		SLink_InitMessage(SLink_MessageTX);
		SLink_SetIdentifier(SLink_MessageTX, 0xDB);
		int32_t errSLink = SLink_LoadPayload(SLink_MessageTX, TXbuffer, TXbufferLen);
		if(errSLink == SLINK_OK)
		{
			SLink_EndMessage(SLink_MessageTX);
			size_t PacketSize = SLink_GetPacketSize(SLink_MessageTX);
			uint8_t *pPacket = SLink_GetPacket(SLink_MessageTX);

			/* Send message */
			int errSocket = Socket_Send(socketfd, remoteAddr, pPacket, PacketSize);
			(void)errSocket; // unused var

			/* Debug Message */
			debug_MessageTx_(SLink_MessageTX);
			SendStats.PacketCount++;
		}
		else
		{
			SendStats.PacketError++;
		}
	}
}

void COMMRecvTask(slink_message_t SLink_MessageRX, int socketfd, struct sockaddr_in *remoteAddr)
{
	uint8_t RXbuffer[RXBUFFER_SIZE];

	size_t RXbufferIdx = 0;
	size_t RXbufferLen = Socket_Recv(socketfd, remoteAddr, RXbuffer, RXBUFFER_SIZE);
	while(RXbufferIdx < RXbufferLen)
	{
		/* Receive message */
		int32_t errSLink = SLink_ReceiveMessageBlock(SLink_MessageRX, RXbuffer, RXbufferLen, &RXbufferIdx);
		if(errSLink == SLINK_OK)
		{
			/* Debug Message */
			debug_MessageRx_(SLink_MessageRX);

			RecvStats.PacketCount++;
		}
		else if(errSLink == SLINK_ERROR)
		{
			RecvStats.PacketError++;
		}
	}
}

/**********************************************************************************************/

int main(int argc, char *argv[])
{
	int option = 0;
	int portNumber = -1;
	char *ipAddress = NULL;
	int timerPeriod = -1;
	struct pollfd pfds[2];
	int socketfd, timerfd, timer2fd, pipefd[2];
	struct sockaddr_in remoteAddr;

	/* Process argv options */
	while ((option = getopt(argc, argv,"hi:p:t:")) != -1)
	{
		switch (option)
		{
		case 'h' :
			print_usage();
			exit(EXIT_SUCCESS);
			break;
		case 'i' :
			ipAddress = strdup(optarg);
			if(strlen(ipAddress) < 7 || strlen(ipAddress) > 21) die("Bad ip address");
			break;
		case 'p' :
			portNumber = atoi(optarg);
			if(portNumber < 0 || portNumber > 65535) die("Bad port");
			break;
		case 't' :
			timerPeriod = atoi(optarg);
			if(timerPeriod < 1) die("Bad timer period");
			break;
		default:
			print_usage();
			exit(EXIT_FAILURE);
		}
	}
	if(ipAddress == NULL) ipAddress = strdup(DEFAULT_REMOTE_ADDR);
	if(portNumber < 0) portNumber = DEFAULT_PORT;
	if(timerPeriod < 0) timerPeriod = DEFAULT_TIMER_PERIOD;
	printf("Info: IP Address  = %s\n", ipAddress);
	printf("Info: Port number = %d\n", portNumber);
	printf("Info: Timer period = %d\n", timerPeriod);
	printf("Info: Timer frequency = %0.3f\n", 1.0f/(timerPeriod / 1000.0f) );
	printf("\r\n");

	memset((char *)&remoteAddr, 0, sizeof(struct sockaddr_in));
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(portNumber);
	remoteAddr.sin_addr.s_addr = inet_addr(ipAddress);

	/* Udp socket */
	socketfd = Socket_Create();
	Socket_Config(socketfd, portNumber+1);

	/* Timer */
	timerfd = Timer_Create();
	Timer_Config(timerfd, timerPeriod);
	timer2fd = Timer_Create();
	Timer_Config(timer2fd, 1000);

	/* Pipes */
	Pipe_Create(pipefd);

	/* Circular buffers */
	circBuffer_Time = SBuffer_Create(TIME_BUFFER_SIZE, TIMESTAMP_SIZE);
	circBuffer_Control = SBuffer_Create(CONTROL_BUFFER_SIZE, CONTROL_SIZE);
	circBuffer_SensorGyro = SBuffer_Create(SENSOR_BUFFER_SIZE, SENSORGYRO_SIZE);
	circBuffer_SensorAccel = SBuffer_Create(SENSOR_BUFFER_SIZE, SENSORACCEL_SIZE);
	circBuffer_SensorMagnet = SBuffer_Create(SENSOR_BUFFER_SIZE, SENSORMAGNET_SIZE);

	/* SLink Messages */
	slink_message_t SLink_MessageTX;
	slink_message_t SLink_MessageRX;
	SLink_MessageRX = SLink_Create(SLINK_MAX_PAYLOAD_SIZE);
	SLink_MessageTX = SLink_Create(SLINK_MAX_PAYLOAD_SIZE);
	SLink_InitMessage(SLink_MessageTX);
	SLink_InitMessage(SLink_MessageRX);
	while(1)
	{
		/* Config fds & poll */
		pfds[0].fd = socketfd;
		pfds[0].events = POLLIN;
		pfds[1].fd = timerfd;
		pfds[1].events = POLLIN;
		pfds[2].fd = timer2fd;
		pfds[2].events = POLLIN;
		int errPoll = poll(pfds, 3, -1);
		if (errPoll < 0) die("poll()");
		else
		{
			if (pfds[0].revents & POLLIN)
			{
				/* Receive task */
				tracepoint(my_provider, my_tracepoint, "Recv-in");
				COMMRecvTask(SLink_MessageRX, socketfd, &remoteAddr);
				tracepoint(my_provider, my_tracepoint, "Recv-out");
			}
			if(pfds[1].revents & POLLIN)
			{
				/* Timer process */
				read(timerfd, NULL, sizeof(uint64_t));
				ControlSimulate(pipefd[1]);

				/* Send Task */
				tracepoint(my_provider, my_tracepoint, "Send-in");
				COMMSendTask(SLink_MessageTX, pipefd[0], socketfd, &remoteAddr);
				tracepoint(my_provider, my_tracepoint, "Send-out");
			}
			if(pfds[2].revents & POLLIN)
			{
				read(timer2fd, NULL, sizeof(uint64_t));
				SendStats.PacketRate = SendStats.PacketCount - SendCount;
				RecvStats.PacketRate = RecvStats.PacketCount - RecvCount;
				SendCount = SendStats.PacketCount;
				RecvCount = RecvStats.PacketCount;

				printf("Send rate: %zu count: %zu error: %zu", SendStats.PacketRate, SendStats.PacketCount, SendStats.PacketError);
				printf(" | ");
				printf("Recv rate: %zu count: %zu error: %zu", RecvStats.PacketRate, RecvStats.PacketCount, RecvStats.PacketError);
				printf("\r\n");
			}
		}
		usleep(10);
	}
	close(socketfd);
	close(timerfd);
	close(timer2fd);
	close(pipefd[0]);
	close(pipefd[1]);
	return 0;
}

/**********************************************************************************************/

void print_usage(void)
{
	printf("demo -i <ip address> -p <port> -t <timer period>\r\n");
}

void die(char *s)
{
	perror(s);
	exit(EXIT_FAILURE);
}

int Socket_Create(void)
{
	int fd;

	/* Create udp socket */
	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) die("socket()");

	/* Set non-blocking flag */
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	return fd;
}

void Socket_Config(int fd, int portNumber)
{
	int error;
	struct sockaddr_in localAddr;

	/* Config udp socket */
	memset((char *)&localAddr, 0, sizeof(struct sockaddr_in));
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(portNumber);
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	error = bind(fd, (struct sockaddr *)&localAddr, sizeof(struct sockaddr_in));
	if (error < 0) die("bind()");
}

int Socket_Send(int fd, struct sockaddr_in *remoteAddr, void *data, size_t size)
{
	socklen_t remoteAddrLen = sizeof(struct sockaddr_in);
	int errSocket = sendto(fd, data, size, 0, (struct sockaddr *) remoteAddr, remoteAddrLen);
	if (errSocket < 0) die("sendto()");
	return errSocket;
}

int Socket_Recv(int fd, struct sockaddr_in *remoteAddr, void *data, size_t size)
{
	socklen_t remoteAddrLen = sizeof(struct sockaddr_in);
	int errSocket = recvfrom(fd, data, size, 0, (struct sockaddr *) remoteAddr, &remoteAddrLen);
	if (errSocket < 0) die("recvfrom()");
	return errSocket;
}

int Timer_Create(void)
{
	int fd;

	/* Create timer */
	fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	if (fd < 0) die("timerfd_create()");
	return fd;
}

void Timer_Config(int fd, uint32_t time)
{
	int error;

	/* Config timer period */
	struct itimerspec new_timer;
	struct itimerspec old_timer;
	new_timer.it_interval.tv_sec = time / 1000;
	new_timer.it_interval.tv_nsec = (time % 1000) * 1000000;
	new_timer.it_value.tv_sec = 0;
	new_timer.it_value.tv_nsec = 1;
	error = timerfd_settime(fd, 0, &new_timer, &old_timer);
	if (error < 0) die("timerfd_settime()");
}

void Pipe_Create(int fd[2])
{
	/* Create pipe */
	int errPipe = pipe(fd);
	if (errPipe < 0) die("pipe()");

	/* Set non-blocking flag */
	int flags = fcntl(fd[0], F_GETFL, 0);
	fcntl(fd[0], F_SETFL, flags | O_NONBLOCK);
}

/**********************************************************************************************/

void debug_MessageTx_(slink_message_t Message)
{
#if defined(DEBUG_MESSAGE_TX)
	uint8_t *pPayload = SLink_GetPayload(Message);
	uint8_t *pPacket = SLink_GetPacket(Message);
	size_t PayloadSize = SLink_GetPayloadSize(Message);
	size_t PacketSize = SLink_GetPacketSize(Message);

	printf("MsgTX:\r\n");
	DEBUG_RawMessagePrint(pPacket, PacketSize);
	DEBUG_SlinkMessagePrint(Message);
	DEBUG_IscomPackedMessagePrint(pPayload, PayloadSize);
	printf("\r\n");
#endif
}

void debug_MessageRx_(slink_message_t Message)
{
#if defined(DEBUG_MESSAGE_RX)
	uint8_t *pPayload = SLink_GetPayload(Message);
	uint8_t *pPacket = SLink_GetPacket(Message);
	size_t PayloadSize = SLink_GetPayloadSize(Message);
	size_t PacketSize = SLink_GetPacketSize(Message);

	printf("MsgRX:\r\n");
	DEBUG_RawMessagePrint(pPacket, PacketSize);
	DEBUG_SlinkMessagePrint(Message);
	DEBUG_IscomPackedMessagePrint(pPayload, PayloadSize);
	printf("\r\n");
#endif
}

