//ZEVA PROTOCOL AT http://zeva.com.au/Products/datasheets/BMS12_CAN_Protocol.pdf

#include <mcp_can.h>
#include <SPI.h>
#include <avr/pgmspace.h>
#include <ubcsolar_can_ids.h>

#define CAN_SS 10
#define BUS_SPEED CAN_125KBPS

bool shouldPrintBMS12Messages = true;
bool shouldPrintCoreStatusMessages = true;
bool shouldPrintCoreConfigMessages = true;
bool shouldSendCoreConfigMessages = true;

MCP_CAN CAN(CAN_SS);

byte bmsAlive = 0;
int numCoreConfigMessagesReceived = 0;

void setup() {  
/* SERIAL INIT */
  Serial.begin(115200);

/* CAN INIT */
  int canSSOffset = 0;

CAN_INIT:

  if(CAN_OK == CAN.begin(BUS_SPEED)) {
    Serial.println(F("CAN BUS Shield init ok!"));
  }
  else {
    Serial.println(F("CAN BUS Shield init fail"));
    Serial.print(F("Init CAN BUS Shield again with SS pin "));
    canSSOffset ^= 1;
    Serial.println(CAN_SS - canSSOffset);
    delay(100);
    CAN = MCP_CAN(CAN_SS - canSSOffset);
    goto CAN_INIT;
  }
}

void printBuf(uint32_t frame_id, byte *frame_data, byte length) {
  Serial.print(F("[Rx] ID: "));
  Serial.print(frame_id,HEX);
        
  Serial.print(F(" Len:"));
  Serial.print(length,HEX);
      
  Serial.print(F(" Data:["));
  for (int i=0;i<length;i++) {
    if (i>0) Serial.print(F(" "));
    Serial.print(frame_data[i],HEX);
  }
  Serial.println(F("]")); 
}

void msgHandler(uint32_t frame_id, byte *frame_data, byte length) {
   
  if(frame_id>=CAN_ID_ZEVA_BMS_BASE && frame_id<CAN_ID_ZEVA_BMS_BASE+40) {
    if(shouldPrintBMS12Messages) {
      msgHandleZevaBms(frame_id, frame_data, length);
    }
  }
  else if(frame_id == CAN_ID_ZEVA_BMS_CORE_STATUS) {
    if(shouldPrintCoreStatusMessages) {
      msgHandleZevaCoreStatus(frame_id, frame_data, length);
    }
    bmsAlive |= B1;
    if(bmsAlive == B1){
      Serial.println("bms alive");
    }
  }
  else if(frame_id >= CAN_ID_ZEVA_BMS_CORE_CONFIG_WR1 && frame_id <= CAN_ID_ZEVA_BMS_CORE_CONFIG_RD3) {
    Serial.println("config packet");
    if(shouldPrintCoreConfigMessages) {
      msgHandleZevaCoreConfig(frame_id, frame_data, length);
    }
    numCoreConfigMessagesReceived++;
    if (numCoreConfigMessagesReceived >= 3) {
      numCoreConfigMessagesReceived = 0;
      bmsAlive |= B1111;
    }
  }
  else {
    Serial.print(F("unknown msg "));
    printBuf(frame_id, frame_data, length);
  }
}

unsigned long timer = 0;

void loop() {
  byte length;
  uint32_t frame_id;
  byte frame_data[8];

  if(CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBuf(&length, frame_data);
    frame_id = CAN.getCanId();
    msgHandler(frame_id, frame_data, length);
  }
  
  if(bmsAlive == B1) {
    zevaCoreSetCellNum();
    bmsAlive |= B11;
    timer = millis() + 1000;
  }

  // enter and exit setup mode to trigger BMS to send current config data
  else if(bmsAlive == B11 && millis() > timer) {
    Serial.println("starting setup");
    zevaCoreStartSetupMode();
    bmsAlive |= B111;
  }
  else if(bmsAlive == B111) {

    if(shouldSendCoreConfigMessages) {
      Serial.println("sending config data");
      zevaCoreSendConfigData1();
      delay(5);
      zevaCoreSendConfigData2();
      delay(5);
      zevaCoreSendConfigData3();
      delay(5);
    }
    
    Serial.println("ending setup");
    zevaCoreEndSetupMode();
    bmsAlive = B11111;
  }
}

