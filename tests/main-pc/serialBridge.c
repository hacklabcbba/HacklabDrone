#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <string.h>

#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <termios.h>
#include <linux/serial.h>

#include "slink.h"
#include "sbuffer.h"

/**********************************************************************************************/

void die(char *s);
void print_usage(void);

int Socket_Create(void);
void Socket_Config(int fd, int portNumber);
int Socket_Send(int fd, struct sockaddr_in *remoteAddr, void *data, size_t size);
int Socket_Recv(int fd, struct sockaddr_in *remoteAddr, void *data, size_t size);
int Serial_Create(char *ttyFile);
void Serial_Config(int fd);
void Serial_ConfigBaudrate(int fd, int ttyBaud);
int Serial_Send(int fd, void *data, size_t size);
int Serial_Recv(int fd, void *data, size_t size);
int Timer_Create(void);
void Timer_Config(int fd, uint32_t time);

/**********************************************************************************************/

#define DEFAULT_REMOTE_ADDR  "10.0.1.200"
#define DEFAULT_TTY_FILE     "/dev/ttyUSB0"
#define DEFAULT_TTY_BAUD     115200
#define DEFAULT_PORT         5000
#define BUFLEN               2048
#define TIMER_PERIOD         1000

/**********************************************************************************************/

int main(int argc, char *argv[])
{
	int option = 0;
	int portNumber = -1;
	char *ttyFile = NULL;
	int ttyBaud = -1;
	struct pollfd pfds[3];
	int serialfd, socketfd, timerfd;
	struct sockaddr_in remoteAddr;

	uint8_t BufferSocket[BUFLEN];
	uint8_t BufferSerial[BUFLEN];

	/* Argv options */
	while ((option = getopt(argc, argv,"ht:b:p:")) != -1)
	{
		switch (option)
		{
		case 'h' :
			print_usage();
			exit(EXIT_SUCCESS);
			break;
		case 't' :
			ttyFile = strdup(optarg);
			break;
		case 'b' :
			ttyBaud = atoi(optarg);
			break;
		case 'p' :
			portNumber = atoi(optarg);
			if(portNumber < 0 || portNumber > 65535) die("Bad port");
			break;
		default:
			print_usage();
			exit(EXIT_FAILURE);
		}
	}
	if(ttyFile == NULL) ttyFile = strdup(DEFAULT_TTY_FILE);
	if(ttyBaud <= 0) ttyBaud = DEFAULT_TTY_BAUD;
	if(portNumber <= 0) portNumber = DEFAULT_PORT;
	printf("Info: Serial Port = %s\n", ttyFile);
	printf("Info: Serial Baudrate = %d\n", ttyBaud);
	printf("Info: Network Port = %d\n", portNumber);

	memset((char *)&remoteAddr, 0, sizeof(struct sockaddr_in));
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(portNumber);
	remoteAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* Udp socket */
	socketfd = Socket_Create();
	Socket_Config(socketfd, portNumber);

	/* Serial port */
	serialfd = Serial_Create(ttyFile);
	Serial_Config(serialfd);
	Serial_ConfigBaudrate(serialfd, ttyBaud);

	/* Timer */
	timerfd = Timer_Create();
	Timer_Config(timerfd, TIMER_PERIOD);

	size_t SerialCount = 0;
	size_t SocketCount = 0;
	slink_statistics_t SerialStats;
	slink_statistics_t SocketStats;
	slink_message_t MessageSocket = SLink_Create(SLINK_MAX_PAYLOAD_SIZE);
	slink_message_t MessageSerial = SLink_Create(SLINK_MAX_PAYLOAD_SIZE);

	/* Config fds & poll */
	pfds[0].fd = socketfd;
	pfds[0].events = POLLIN;
	pfds[1].fd = serialfd;
	pfds[1].events = POLLIN;
	pfds[2].fd = timerfd;
	pfds[2].events = POLLIN;
	while(1)
	{
		int errPoll = poll(pfds, 3, 10);
		if (errPoll < 0) die("poll()");
		else
		{
			/* Receive socket message */
			if (pfds[0].revents & POLLIN)
			{
				pfds[0].revents = 0;
				struct sockaddr_in remoteAddrTmp = remoteAddr;
				size_t BufferSocketIdx = 0;
				size_t BufferSocketLen = Socket_Recv(socketfd, &remoteAddrTmp, BufferSocket, BUFLEN);
				while(BufferSocketIdx < BufferSocketLen)
				{
					/* Receive message */
					int32_t errSLink = SLink_ReceiveMessageBlock(MessageSocket, BufferSocket, BufferSocketLen, &BufferSocketIdx);
					if(errSLink == SLINK_OK)
					{
						remoteAddr = remoteAddrTmp;
						uint8_t *pPacket = SLink_GetPacket(MessageSocket);
						size_t PacketSize = SLink_GetPacketSize(MessageSocket);
						int errSerial = Serial_Send(serialfd, pPacket, PacketSize);
						(void)errSerial; // unused var
						SocketStats.PacketCount++;
					}
					else if(errSLink == SLINK_ERROR)
					{
						SocketStats.PacketError++;
					}
				}
			}

			/* Receive serial message */
			if(pfds[1].revents & POLLIN)
			{
				pfds[1].revents = 0;
				size_t BufferSerialIdx = 0;
				size_t BufferSerialLen = Serial_Recv(serialfd, BufferSerial, BUFLEN);
				while(BufferSerialIdx < BufferSerialLen)
				{
					/* Receive message */
					int32_t errSLink = SLink_ReceiveMessageBlock(MessageSerial, BufferSerial, BufferSerialLen, &BufferSerialIdx);
					if(errSLink == SLINK_OK)
					{
						uint8_t *pPacket = SLink_GetPacket(MessageSerial);
						size_t PacketSize = SLink_GetPacketSize(MessageSerial);
						int errSocket = Socket_Send(socketfd, &remoteAddr, pPacket, PacketSize);
						(void)errSocket; // unused var
						SerialStats.PacketCount++;
					}
					else if(errSLink == SLINK_ERROR)
					{
						SerialStats.PacketError++;
					}
				}
			}

			/* Timer process */
			if(pfds[2].revents & POLLIN)
			{
				pfds[2].revents = 0;
				read(timerfd, NULL, sizeof(uint64_t));
				SocketStats.PacketRate = SocketStats.PacketCount - SocketCount;
				SerialStats.PacketRate = SerialStats.PacketCount - SerialCount;
				SocketCount = SocketStats.PacketCount;
				SerialCount = SerialStats.PacketCount;
				printf("Socket rate: %zu count: %zu error: %zu", SocketStats.PacketRate, SocketStats.PacketCount, SocketStats.PacketError);
				printf(" | ");
				printf("Serial rate: %zu count: %zu error: %zu", SerialStats.PacketRate, SerialStats.PacketCount, SerialStats.PacketError);
				printf("\r\n");
			}
		}
	}
	close(serialfd);
	close(socketfd);
	close(timerfd);
	return 0;
}

/**********************************************************************************************/

void print_usage(void)
{
	printf("serialBridge -t <tty> -b <baudrate> -p <port> \r\n");
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

int Serial_Create(char *ttyFile)
{
	int fd;

	/* Open serial port */
	fd = open(ttyFile, O_RDWR | O_NOCTTY);
	if (fd < 0) die("open()");

	/* Set non-blocking flag */
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	return fd;
}

void Serial_Config(int fd)
{
	struct termios toptions;

	/* Get current serial port settings */
	tcgetattr(fd, &toptions);

	/* Non Canonical mode */
	/* Disable software flow control */
	/* Disable hardware flow control */
	/* Enable receiver, ignore status lines */
	/* 8 bits, no parity, 1 stop bits */
	toptions.c_cflag = CS8 | CREAD | CLOCAL;
	toptions.c_iflag=0;
	toptions.c_oflag=0;
	toptions.c_lflag=0;
	toptions.c_cc[VMIN] = 1;
	toptions.c_cc[VTIME] = 0;

	/* Set baudrate */
	cfsetispeed(&toptions, B115200);
	cfsetospeed(&toptions, B115200);

	/* Commit the serial port settings */
	tcsetattr(fd, TCSANOW, &toptions);

	struct serial_struct serinfo;
	ioctl (fd, TIOCGSERIAL, &serinfo);
	serinfo.flags |= 0x4000;
	ioctl (fd, TIOCSSERIAL, &serinfo);

	/* Flush anything already in the serial buffer */
	tcflush(fd, TCIFLUSH);
}

void Serial_ConfigBaudrate(int fd, int ttyBaud)
{
	struct termios toptions;
	int baudrate;

	/* Get current serial port settings */
	tcgetattr(fd, &toptions);

	switch(ttyBaud)
	{
	case 9600:
		baudrate = B9600;
		break;
	case 19200:
		baudrate = B19200;
		break;
	case 38400:
		baudrate = B38400;
		break;
	case 57600:
		baudrate = B57600;
		break;
	case 115200:
		baudrate = B115200;
		break;
	case 230400:
		baudrate = B230400;
		break;
	case 460800:
		baudrate = B460800;
		break;
	case 500000:
		baudrate = B500000;
		break;
	case 576000:
		baudrate = B576000;
		break;
	case 921600:
		baudrate = B921600;
		break;
	case 1000000:
		baudrate = B1000000;
		break;
	case 1152000:
		baudrate = B1152000;
		break;
	case 1500000:
		baudrate = B1500000;
		break;
	case 2000000:
		baudrate = B2000000;
		break;
	case 2500000:
		baudrate = B2500000;
		break;
	case 3000000:
		baudrate = B3000000;
		break;
	case 3500000:
		baudrate = B3500000;
		break;
	case 4000000:
		baudrate = B4000000;
		break;
	default:
		die("Wrong baudrate");
	}

	/* Set baudrate */
	cfsetispeed(&toptions, baudrate);
	cfsetospeed(&toptions, baudrate);

	/* Commit the serial port settings */
	tcsetattr(fd, TCSANOW, &toptions);
}

int Serial_Send(int fd, void *data, size_t size)
{
	int errSerial = write(fd, data, size);
	if (errSerial < 0) die("write()");
	return errSerial;
}

int Serial_Recv(int fd, void *data, size_t size)
{
	int errSerial = read(fd, data, size);
	if (errSerial < 0) die("read()");
	return errSerial;
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

