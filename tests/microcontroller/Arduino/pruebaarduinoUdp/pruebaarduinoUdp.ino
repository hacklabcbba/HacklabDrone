#include <Arduino.h>
#include "slink.h"

#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008

#include "transport_types.h"
#include "iscom.h"

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress localIp(192, 168, 1, 177);
unsigned int localPort = 5000;      // local port to listen on
IPAddress remoteIp;
unsigned int remotePort;
int linkState = 0;
int count = 0;

// buffers for receiving and sending data
char RXbuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet
char TXbuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;
slink_message_t MessageTX;

SensorAccel_t acelerometro;
iscom_message_t Message;

void setup(){
  MessageTX = SLink_Create(200);
  Serial.begin(115200);
  // start the Ethernet and UDP:
  Ethernet.begin(mac, localIp);
  Udp.begin(localPort);
}

void loop(){
  static uint32_t lastTime, currentTime = 0;
  
  //Comunicacion por Ethernet
  //Recibir
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    linkState = 1;
    count++;
    
    remoteIp = Udp.remoteIP();
    remotePort = Udp.remotePort();
    Udp.read(RXbuffer, UDP_TX_PACKET_MAX_SIZE);
    //debugRecieve(packetSize, count);
  }

  //Enviar
  if(linkState)
  {
    currentTime = micros();
    if(currentTime - lastTime > 10000)
    {  
      lastTime = currentTime;
      acelerometro.data[0].elem[0] = analogRead(A3);
      acelerometro.data[0].elem[1] = analogRead(A4);
      acelerometro.data[0].elem[2] = analogRead(A5);
      
      ISCOM_DeInit(&Message);
      ISCOM_SetContent(&Message, &acelerometro, SENSORACCEL_SIZE);
      ISCOM_SetIdentifier(&Message, 0x01, 0x1B);
      
      size_t offsetTotal = 0;
      for(int i=0; i<1; i++)
      {
        size_t offset = ISCOM_Pack(&Message, TXbuffer + offsetTotal);
        offsetTotal += offset;
      }
      
      SLink_InitMessage(MessageTX);
      SLink_SetIdentifier(MessageTX, 0xAB);
      SLink_LoadPayload(MessageTX, TXbuffer, offsetTotal);
      SLink_EndMessage(MessageTX);
  
      uint8_t *buffertx = (uint8_t*)SLink_GetPacket(MessageTX);
      size_t bufferlen = SLink_GetPacketSize(MessageTX);
      
      Udp.beginPacket(remoteIp, remotePort);
      Udp.write(buffertx, bufferlen);
      Udp.endPacket();
    }
  }
}

void debugRecieve(int packetSize, int count){
    Serial.print("Received packet, Count=");
    Serial.print(count);
    Serial.print(", Size=");
    Serial.println(packetSize);
    Serial.print("From ");
    for (int i = 0; i < 4; i++) {
      Serial.print(remoteIp[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(remotePort);
}

