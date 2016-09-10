#include <Arduino.h>
#include "slink.h"

#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);
unsigned int localPort = 8888;      // local port to listen on
// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet,
char  ReplyBuffer[] = "acknowledged";       // a string to send back
// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;


SLink_Message_t MessageTX;
uint8_t *buffertx = (uint8_t*)SLink_GetPacket(MessageTX);
size_t bufferlen = SLink_GetPacketSize(MessageTX);
uint8_t var = 0x12;

void setup(){
  MessageTX = SLink_Create(100);
  Serial.begin(9600);
  // start the Ethernet and UDP:
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);

}

void loop(){

  SLink_InitMessage(MessageTX);
  SLink_SetIdentifier(MessageTX, 0xAB);
  SLink_LoadPayload(MessageTX, &var, 1);
  SLink_EndMessage(MessageTX);

  buffertx = (uint8_t*)SLink_GetPacket(MessageTX);
  bufferlen = SLink_GetPacketSize(MessageTX);

  //Comunicacion por Ethernet
  // if there's data available, read a packet
  //int packetSize = Udp.parsePacket();
  if(bufferlen){ //if (packetSize) {
    Serial.print("Received packet of size ");
    //Serial.println(packetSize);
    Serial.println(bufferlen);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i = 0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    //Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Udp.read(MessageTX, bufferlen);
    Serial.println("Contents:");
    //Serial.println(packetBuffer);
    Serial.println(Messagetx);

    // send a reply to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBuffer);
    Udp.endPacket();
  }

  //Comunicación Serial
  for(int i = 0; i< bufferlen; i++){
    uint8_t dato = buffertx[i];
    Serial.print(dato,HEX);
    Serial.print(" ");
    }
    Serial.println();
    
delay(5000);
}


