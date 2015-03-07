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

#define BUS_SPEED 125

BMSConfig bmsConfig = {0}; //set valid to 0

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
  byte voltGrp=(frame_id%10)/2;
  int vx100[6];
  int temp;
  for(int i=0; i<6; i++) {
    vx100[i]=frame_data[i]+((frame_data[6]>>i)&1 ? 256 : 0);
  }
  temp = frame_data[7] - 128;
  
  Serial.print("BMS #");
  Serial.print(bmsId);
  for(int i=0; i<6; i++) {
    Serial.print(" c");
    Serial.print(i+6*(voltGrp-1));
    Serial.print("=");
    Serial.print(vx100[i]);
  }
  Serial.print(" t");
  Serial.print(voltGrp);
  Serial.print("=");
  Serial.print(temp);
  Serial.println();
}

void msgHandler(byte rx_status, byte length, uint32_t frame_id, byte filter, byte buffer, byte *frame_data, byte ext) {
   
   if(frame_id>=ZEVA_BMS_BASE_ID && frame_id<ZEVA_BMS_BASE_ID+40) {
     msgHandleZevaBms(rx_status, length, frame_id, filter, buffer, frame_data, ext);
   }
   else if(frame_id == ZEVA_BMS_CORE_STATUS) {
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
}

