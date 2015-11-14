//Code by Patrick Cruce(pcruce_at_igpp.ucla.edu) 
//Uses CAN extended library, designed for Arduino with 16 Mhz oscillator
//Library found at: http://code.google.com/p/canduino/source/browse/trunk/#trunk%2FLibrary%2FCAN
//This runs a simple test of the hardware with the MCP 2515 CAN Controller in loopback mode.
//If using over physical bus rather than loopback, and you have high bus 
//utilization, you'll want to turn the baud of the serial port up or log
//to SD card, because frame drops can occur due to the reader code being
//occupied with writing to the port.
//In our testing over a 40 foot cable, we didn't have any problems with
//synchronization or frame drops due to SPI,controller, or propagation delays
//even at 1 Megabit.  But we didn't do any tests that required arbitration
//with multiple nodes.

//ZEVA PROTOCOL AT http://zeva.com.au/Products/datasheets/BMS12_CAN_Protocol.pdf

#include <CAN.h>
#include <SoftwareSerial.h>
#include <SPI.h>

#include "bms_defs.h"
#include <ubcsolar_can_ids.h>

#define DEBUG 0 //print contents of all packets received instead of once per second
#define PRINT_JSON 1 //print data in json format. only when DEBUG=0

#define BUS_SPEED 125
#define PRINT_DELAY 1000

BMSConfig bmsConfig = {0}; //set valid to 0
BMSStatus bmsStatus = {0};
int cellVoltagesX100[4][12] = {{0}};
signed char bmsTemperatures[4][2] = {{0}};

unsigned char bmsAlive = 0;
unsigned long int lastPrintTime = 0;

void setup() {  
  
  Serial.begin(115200);
  
  // initialize CAN bus class
  // this class initializes SPI communications with MCP2515

  CAN.begin();
  
  CAN.setMode(CONFIGURATION);
  CAN.baudConfig(BUS_SPEED);
  CAN.toggleRxBuffer0Acceptance(true, true); //set to true,true to disable filtering
  CAN.toggleRxBuffer1Acceptance(true, true);
  CAN.setMode(NORMAL);  // set to "NORMAL" for standard com

  /* set mask bit to 1 to turn on filtering for that bit
                                                   un       DATA        DATA
                                  ID MSB   ID LSB used     BYTE 0      BYTE 1
                                  [             ][   ]    [      ]    [      ]  */                                
  CAN.setMaskOrFilter(MASK_0,   0b00000000, 0b00000000, 0b00000000, 0b00000000); //mask 0 (for buffer 0) controls filters 0 and 1 
  CAN.setMaskOrFilter(FILTER_0, 0b00000000, 0b00000000, 0b00000000, 0b00000000); //to pass a filter, all bits in the msg id that are "masked" must be the same as in the filter.
  CAN.setMaskOrFilter(FILTER_1, 0b00000000, 0b00000000, 0b00000000, 0b00000000); //a message will pass if at least one of the filters pass it. 
  CAN.setMaskOrFilter(MASK_1,   0b00000000, 0b00000000, 0b00000000, 0b00000000); //mask 1 (for buffer 1) controls filters 2 to 5
  CAN.setMaskOrFilter(FILTER_2, 0b00000000, 0b00000000, 0b00000000, 0b00000000);
  CAN.setMaskOrFilter(FILTER_3, 0b00000000, 0b00000000, 0b00000000, 0b00000000);
  CAN.setMaskOrFilter(FILTER_4, 0b00000000, 0b00000000, 0b00000000, 0b00000000); //shows up as 0 on printBuf
  CAN.setMaskOrFilter(FILTER_5, 0b00000000, 0b00000000, 0b00000000, 0b00000000); //shows up as 1 on printBuf
}

void printBuf(byte rx_status, byte length, uint32_t frame_id, byte filter, byte buffer, byte *frame_data, byte ext) {
     
  Serial.print("[Rx] Status:");
  Serial.print(rx_status,HEX);
        
  Serial.print(" Len:");
  Serial.print(length,HEX);
      
  Serial.print(" Frame:");
  Serial.print(frame_id,HEX);

  Serial.print(" EXT?:");
  Serial.print(ext==1,HEX);
       
  Serial.print(" Filter:");
  Serial.print(filter,HEX);

  Serial.print(" Buffer:");
  Serial.print(buffer,HEX);
      
  Serial.print(" Data:[");
  for (int i=0;i<length;i++) {
    if (i>0) Serial.print(" ");
    Serial.print(frame_data[i],HEX);
  }
  Serial.println("]"); 
}

void msgHandleZevaBms(byte rx_status, byte length, uint32_t frame_id, byte filter, byte buffer, byte *frame_data, byte ext) {
  
  uint32_t messageID = frame_id%10;
  
  // even IDs are requests from Core to BMS12
  if(messageID%2 == 0) {
    #if DEBUG
    switch (messageID) {
      case 0: Serial.print("Request for status from Core to Module "); break;
      case 2: Serial.print("Request for voltages #1 from Core to Module "); break;
      case 4: Serial.print("Request for voltages #2 from Core to Module "); break;
      case 6: Serial.print("Request for config info from Core to Module "); break;
      case 8: Serial.print("Command to set config info from Core to Module "); break;
      default: Serial.print("Unknown message from Core to Module "); break;
    }
    Serial.print((frame_id - 100 - messageID)/10);  // frameID = 100 + 10 * moduleID + messageID, 0 <= messageID <=8
    Serial.println();
    #endif
    return;
  }
  
  if(messageID != 3 && messageID != 5){
    Serial.print("BMS #");
    Serial.print((frame_id-100)/10);
    Serial.print(" packet ");
    Serial.print(frame_id%10);
    Serial.println(" parsing not implemented");
    return;
  }
  
  byte bmsId=(frame_id-100)/10;
  byte voltGrp=(frame_id%10)/2-1;
  
  for(int i=0; i<6; i++) {
    cellVoltagesX100[bmsId][voltGrp*6+i] = frame_data[i]+((frame_data[6]>>i)&1 ? 256 : 0);
  }
  bmsTemperatures[bmsId][voltGrp] = frame_data[7] - 128;
  
  #if DEBUG
  Serial.print("BMS #");
  Serial.print(bmsId);
  for(int i=0; i<6; i++) {
    Serial.print(" c");
    Serial.print(i+6*voltGrp);
    Serial.print("=");
    Serial.print(cellVoltagesX100[bmsId][voltGrp*6+i]);
  }
  Serial.print(" t");
  Serial.print(voltGrp);
  Serial.print("=");
  Serial.print(bmsTemperatures[bmsId][voltGrp]);
  Serial.println();
  #endif
}

void msgHandler(byte rx_status, byte length, uint32_t frame_id, byte filter, byte buffer, byte *frame_data, byte ext) {
   
   if(frame_id>=CAN_ID_ZEVA_BMS_BASE && frame_id<CAN_ID_ZEVA_BMS_BASE+40) {
     msgHandleZevaBms(rx_status, length, frame_id, filter, buffer, frame_data, ext);
   }
   else if(frame_id == CAN_ID_ZEVA_BMS_CORE_STATUS) {
     msgHandleZevaCoreStatus(rx_status, length, frame_id, filter, buffer, frame_data, ext);
   }
   else if(frame_id >= 12 && frame_id <= 17) {
     msgHandleZevaCoreConfig(rx_status, length, frame_id, filter, buffer, frame_data, ext);
   }
   else {
     Serial.print("unknown msg ");
     printBuf(rx_status, length, frame_id, filter, buffer, frame_data, ext);     
   }
}

void loop() {
  
  byte length,rx_status,filter,ext;
  uint32_t frame_id;
  byte frame_data[8];
 
  // Rx
  // clear receive buffers, just in case.
  for(int i=0;i<8;i++)  frame_data[i] = 0x00;
      
  frame_id = 0x0000; 
  length = 0;
      
  rx_status = CAN.readStatus();

  if ((rx_status & 0x40) == 0x40) {

    CAN.readDATA_ff_0(&length,frame_data,&frame_id, &ext, &filter);
    //printBuf(rx_status, length, frame_id, filter, 0, frame_data, ext);
    msgHandler(rx_status, length, frame_id, filter, 0, frame_data, ext);
    CAN.clearRX0Status();
    rx_status = CAN.readStatus();
    //Serial.println(rx_status,HEX);
  }
      
  if ((rx_status & 0x80) == 0x80) {

    CAN.readDATA_ff_1(&length,frame_data,&frame_id, &ext, &filter);
    //printBuf(rx_status, length, frame_id, filter, 1, frame_data, ext);       
    msgHandler(rx_status, length, frame_id, filter, 1, frame_data, ext);
    CAN.clearRX1Status();
    rx_status = CAN.readStatus();
    //Serial.println(rx_status,HEX);
  }
  
  if(bmsAlive == 1){
    zevaCoreSetCellNum();
    bmsAlive |= 2;
  }
  
#if !DEBUG
  if(millis() - lastPrintTime > PRINT_DELAY){
    lastPrintTime += PRINT_DELAY;
#if !PRINT_JSON
    zevaCoreStatusPrint();
    for(int i=0; i<4; i++){
      int total = 0;
      Serial.print("BMS #");
      Serial.print(i);
      Serial.print(": ");
      for(int j=0; j<12; j++){
        total += cellVoltagesX100[i][j];
        Serial.print("c");
        Serial.print(j);
        Serial.print("=");
        Serial.print(cellVoltagesX100[i][j]);
        Serial.print(" ");
      }
      for(int j=0; j<2; j++){
        Serial.print("t");
        Serial.print(j);
        Serial.print("=");
        Serial.print(bmsTemperatures[i][j]);
        Serial.print(" ");
      }
      Serial.print("V=");
      Serial.print(total / 100.0);
      Serial.println();
    }
#else
    Serial.print("{\"speed\":");
    Serial.print(0); //dummy value for now
    Serial.print(",\"totalVoltage\":");
    Serial.print(bmsStatus.voltage);
    Serial.print(",\"stateOfCharge\":");
    Serial.print(bmsStatus.soc);
    Serial.print(",\"temperatures\":{\"bms\":");
    Serial.print(bmsStatus.temperature);
    Serial.print(",\"motor\":");
    Serial.print(1); //dummy value for now
    for(int i=0; i<4; i++){
      Serial.print(",\"pack");
      Serial.print(i);
      Serial.print("\":");
      Serial.print(bmsTemperatures[i][0]);
    }
    Serial.print("}");
    Serial.print(",\"cellVoltages\":{");
    bool firstValI = true;
    bool firstValJ = true;
    for(int i=0; i<4; i++){
      if(!firstValI)
        Serial.print(",");
      firstValI = false;
      Serial.print("\"pack");
      Serial.print(i);
      Serial.print("\":[");
      firstValJ = true;
      for(int j=0; j<12; j++){
        if(!firstValJ)
          Serial.print(",");
        firstValJ = false;
        Serial.print(cellVoltagesX100[i][j]/100.0);
      }
      Serial.print("]");
    }
    Serial.print("}}\n");
#endif
  }
#endif
}

