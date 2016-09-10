/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Global variables ---------------------------------------------------------*/
DhcpClientSettings dhcpClientSettings;
DhcpClientCtx dhcpClientContext;

AutoIpContext aIpContext;
AutoIpSettings aIpSettings;

TaskHandle_t xSensorManagerTask;
TaskHandle_t xCOMMManagerTask;
TaskHandle_t xControlTask;
QueueHandle_t queue_sensors;
QueueHandle_t queue_control;
SemaphoreHandle_t mutex_hal;
SemaphoreHandle_t mutex_spi;
SemaphoreHandle_t semap_ctrl;
slink_message_t SLink_MessageTX;
slink_message_t SLink_MessageRX;
sbuffer_t circBuffer_Control;
sbuffer_t circBuffer_SensorGyro;
sbuffer_t circBuffer_SensorAccel;
sbuffer_t circBuffer_SensorMagnet;
timestamp_t localTime;
timestamp_t remoteTime;
volatile uint32_t linkStatus;
IpAddr remoteAddr;
uint16_t remotePort;

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void SystemInitAll(void);

void SensorManagerTask(void *param);
void COMMManagerTask(void *param);
void ControlTask(void *param);
void BlinkTask(void *param);
void SNTPTask(void *param);

/* Private function prototypes -----------------------------------------------*/
int main(void)
{
	NetInterface *interface;
	TaskHandle_t task;
	MacAddr macAddr;
	Ipv4Addr ipv4Addr;
	error_t error;

	//Initialize system
	SystemInitAll();

	//Configure debug UART
	debugInit(115200);

	//Start-up message
	TRACE_INFO("\r\n");
	TRACE_INFO("***************************\r\n");
	TRACE_INFO("***  CycloneTCP Server  ***\r\n");
	TRACE_INFO("***************************\r\n");
	TRACE_INFO("Copyright: 2010-2016 Oryx Embedded SARL\r\n");
	TRACE_INFO("Compiled: %s %s\r\n", __DATE__, __TIME__);
	TRACE_INFO("Target: STM32F429ZI\r\n");
	TRACE_INFO("\r\n");

	//Initialize kernel & trace
	vTraceInitTraceData();
	osInitKernel();
	TRACE_INFO("Initializing Trace...\r\n");

	//TCP/IP stack initialization
	error = netInit();
	if(error)
		TRACE_ERROR("Failed to initialize TCP/IP stack!\r\n");

	//Configure the first Ethernet interface
	interface = &netInterface[0];

	//Set interface name
	netSetInterfaceName(interface, "eth0");
	//Set host name
	netSetHostname(interface, "ServerDemo");
	//Select the relevant network adapter
	netSetDriver(interface, &stm32EthDriver);
	netSetPhyDriver(interface, &lan8742PhyDriver);
	//Set host MAC address
	macStringToAddr(APP_MAC_ADDR, &macAddr);
	netSetMacAddr(interface, &macAddr);

	//Initialize network interface
	error = netConfigInterface(interface);
	if(error)
		TRACE_ERROR("Failed to configure interface %s!\r\n", interface->name);

#if(APP_USE_DHCP == ENABLED)

	//Get default settings
	dhcpClientGetDefaultSettings(&dhcpClientSettings);
	//Set the network interface to be configured by DHCP
	dhcpClientSettings.interface = interface;
	//Disable rapid commit option
	dhcpClientSettings.rapidCommit = FALSE;

	//DHCP client initialization
	error = dhcpClientInit(&dhcpClientContext, &dhcpClientSettings);
	if(error)
		TRACE_ERROR("Failed to initialize DHCP client!\r\n");

	//Start DHCP client
	error = dhcpClientStart(&dhcpClientContext);
	if(error)
		TRACE_ERROR("Failed to start DHCP client!\r\n");

#elif(APP_USE_AUTOIP == ENABLED)

	//Get default settings
	autoIpGetDefaultSettings(&aIpSettings);
	//Set the network interface to be configured by Auto-Ip
	aIpSettings.interface = interface;
	//Auto-Ip initialization
	error = autoIpInit(&aIpContext, &aIpSettings);
	if(error)
		TRACE_ERROR("Failed to initialize Auto-Ip!\r\n");

	//Start Auto-Ip
	error = autoIpStart(&aIpContext);
	if(error)
		TRACE_ERROR("Failed to start Auto-Ip!\r\n");

#else

	//Set IPv4 host address
	ipv4StringToAddr(APP_IPV4_HOST_ADDR, &ipv4Addr);
	ipv4SetHostAddr(interface, ipv4Addr);

	//Set subnet mask
	ipv4StringToAddr(APP_IPV4_SUBNET_MASK, &ipv4Addr);
	ipv4SetSubnetMask(interface, ipv4Addr);

	//Set default gateway
	ipv4StringToAddr(APP_IPV4_DEFAULT_GATEWAY, &ipv4Addr);
	ipv4SetDefaultGateway(interface, ipv4Addr);

	TRACE_INFO("Network configuration:\r\n");
	//Host address
	TRACE_INFO("  IPv4 Address = %s\r\n", ipv4AddrToString(interface->ipv4Context.addr, NULL));
	//Subnet mask
	TRACE_INFO("  Subnet Mask = %s\r\n", ipv4AddrToString(interface->ipv4Context.subnetMask, NULL));
	//Default gateway
	TRACE_INFO("  Default Gateway = %s\r\n", ipv4AddrToString(interface->ipv4Context.defaultGateway, NULL));
	//DNS servers
	for(uint32_t i = 0; i < IPV4_DNS_SERVER_LIST_SIZE; i++)
	{
		TRACE_INFO("  DNS Server %u = %s\r\n", i + 1, ipv4AddrToString(interface->ipv4Context.dnsServerList[i], NULL));
	}
	//Maximum transmit unit
	TRACE_INFO("  MTU = %" PRIuSIZE "\r\n", interface->ipv4Context.linkMtu);
	TRACE_INFO("\r\n");

#endif

	//Trace recorder start
	error = uiTraceStart();
	if(!error)
		TRACE_ERROR("Failed to start trace recorder!\r\n");

	//Create SLink Messages
	SLink_MessageTX = SLink_Create(SLINK_MAX_PAYLOAD_SIZE);
	SLink_MessageRX = SLink_Create(SLINK_MAX_PAYLOAD_SIZE);

	//Test SLink Messages
	configASSERT(SLink_MessageTX != NULL);
	configASSERT(SLink_MessageRX != NULL);

	//Init SLink Messages
	SLink_InitMessage(SLink_MessageTX);
	SLink_InitMessage(SLink_MessageRX);

	//Create Circular Buffers
	circBuffer_Control = SBuffer_Create(CONTROL_BUFFER_SIZE, CONTROL_SIZE);
	circBuffer_SensorGyro = SBuffer_Create(SENSOR_BUFFER_SIZE, SENSORGYRO_SIZE);
	circBuffer_SensorAccel = SBuffer_Create(SENSOR_BUFFER_SIZE, SENSORACCEL_SIZE);
	circBuffer_SensorMagnet = SBuffer_Create(SENSOR_BUFFER_SIZE, SENSORMAGNET_SIZE);

	//Test Circular Buffers
	configASSERT(circBuffer_Control != NULL);
	configASSERT(circBuffer_SensorGyro != NULL);
	configASSERT(circBuffer_SensorAccel != NULL);
	configASSERT(circBuffer_SensorMagnet != NULL);

	//Create Queues
	queue_sensors = xQueueCreate(SENSOR_BUFFER_SIZE, ISCOM_MESSAGE_SIZE);
	queue_control = xQueueCreate(CONTROL_BUFFER_SIZE, ISCOM_MESSAGE_SIZE);

	//Test Queues
	configASSERT(queue_sensors != NULL);
	configASSERT(queue_control != NULL);

	//Create Semaphores
	semap_ctrl = xSemaphoreCreateBinary();

	//Test Semaphores
	configASSERT(semap_ctrl != NULL);
	xSemaphoreTake(semap_ctrl, 0);

	//Create Mutexes
	mutex_hal = xSemaphoreCreateMutex();
	mutex_spi = xSemaphoreCreateMutex();

	//Test Mutexes
	configASSERT(mutex_hal != NULL);
	configASSERT(mutex_spi != NULL);

	//Create Tasks
	xTaskCreate(SensorManagerTask, "SensorManager", 500, NULL, configMAX_PRIORITIES - 1, &xSensorManagerTask);
	xTaskCreate(COMMManagerTask, "COMMManager", 1000, NULL, configMAX_PRIORITIES - 2, &xCOMMManagerTask);
	xTaskCreate(ControlTask, "ControlTask", 500, NULL, configMAX_PRIORITIES - 5, &xControlTask);
	xTaskCreate(BlinkTask, "Blink", 500, NULL, tskIDLE_PRIORITY, NULL);
	//	task = osCreateTask("SNTP", SNTPTask, NULL, 500, 7);
	vTaskStartScheduler();
	while(1)
		;
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}

#endif

/**
  * @}
  */

/**
  * @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/