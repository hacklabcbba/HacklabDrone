#ifndef __MAIN
#define __MAIN

#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "trcUser.h"

#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"

#include "hal.h"
#include "iscom.h"
#include "sbuffer.h"
#include "slink.h"
#include "transport_types.h"
#include "globalVar.h"

#include "gtime.h"
#include "serial.h"

//Application configuration
#define BAUD_115200    (115200)
#define BAUD_921600    (921600)
#define BAUD_1000000   (1000000)
#define BAUD_2000000   (2000000)
#define BAUD_3000000   (3000000)
#define BAUD_4000000   (4000000)
#define SLINK_BAUDRATE (BAUD_2000000)

#define SENSOR_TIME_PERIOD  (2)
#define SENSOR_BUFFER_SIZE  (64)
#define CONTROL_BUFFER_SIZE (1)

#endif