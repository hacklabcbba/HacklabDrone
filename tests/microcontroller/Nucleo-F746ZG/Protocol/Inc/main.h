#ifndef __MAIN
#define __MAIN

#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "trcUser.h"

#include "debug.h"
#include "os_port.h"
#include "core/net.h"
#include "dhcp/dhcp_client.h"
#include "sntp/sntp_client.h"

#include "stm32f7xx_hal.h"
#include "drivers/lan8742.h"
#include "drivers/stm32f7xx_eth.h"

#include "iscom.h"
#include "sbuffer.h"
#include "slink.h"
#include "transport_types.h"
#include "globalVar.h"

#include "gtime.h"

//Application configuration
//#define APP_USE_DHCP             ENABLED
//#define APP_USE_AUTOIP           ENABLED
#define APP_MAC_ADDR "48-44-72-6F-6E-65"
#define APP_IPV4_HOST_ADDR "169.254.1.1"
#define APP_IPV4_SUBNET_MASK "255.255.255.0"
#define APP_IPV4_DEFAULT_GATEWAY "0.0.0.0"
#define APP_IPV4_HOST_PORT 5000

#define SENSOR_TIME_PERIOD 1
#define SENSOR_BUFFER_SIZE 64
#define CONTROL_BUFFER_SIZE 1

#define stm32EthDriver stm32f7xxEthDriver

#endif