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

#define BUS_SPEED 125

#define CAN_ID_SIGNAL_CTRL 9
#define CAN_ID_MOTOR_CTRL 4

byte throttle = 0;
byte regen = 0;
byte dir = 0;


void setup() {  
  
  Serial.begin(115200);
  
  // initialize CAN bus class
  // this class initializes SPI communications with MCP2515

  CAN.begin();
  
  CAN.setMode(CONFIGURATION);
  CAN.baudConfig(BUS_SPEED);
  CAN.toggleRxBuffer0Acceptance(true, false); //set to true,true to disable filtering
  CAN.toggleRxBuffer1Acceptance(false, false);
  CAN.setMode(NORMAL);  // set to "NORMAL" for standard com

  /* set mask bit to 1 to turn on filtering for that bit
                                                   un       DATA        DATA
                                  ID MSB   ID LSB used     BYTE 0      BYTE 1
                                  [             ][   ]    [      ]    [      ]  */                                
  CAN.setMaskOrFilter(MASK_0,   0b11111111, 0b11100000, 0b00000000, 0b00000000); //mask 0 (for buffer 0) controls filters 0 and 1 
  CAN.setMaskOrFilter(FILTER_0, 0b00001000, 0b10000000, 0b00000000, 0b00000000); //to pass a filter, all bits in the msg id that are "masked" must be the same as in the filter.
  CAN.setMaskOrFilter(FILTER_1, 0b00000001, 0b00100000, 0b00000000, 0b00000000); //a message will pass if at least one of the filters pass it. 
  CAN.setMaskOrFilter(MASK_1,   0b11111111, 0b11100000, 0b00000000, 0b00000000); //mask 1 (for buffer 1) controls filters 2 to 5
  CAN.setMaskOrFilter(FILTER_2, 0b00000000, 0b10000000, 0b00000000, 0b00000000);
  CAN.setMaskOrFilter(FILTER_3, 0b00000001, 0b00100000, 0b00000000, 0b00000000);
  CAN.setMaskOrFilter(FILTER_4, 0b00000000, 0b00000000, 0b00000000, 0b00000000); //shows up as 0 on printBuf
  CAN.setMaskOrFilter(FILTER_5, 0b00000000, 0b00000000, 0b00000000, 0b00000000); //shows up as 1 on printBuf
  
  // we only want 4 (motor ctrl) and 9 (light ctrl)
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

void msgHandleMotorCtrl(byte rx_status, byte length, uint32_t frame_id, byte filter, byte buffer, byte *frame_data, byte ext){
  throttle = frame_data[0];
  regen = frame_data[1];
  dir = frame_data[2];
  Serial.print(throttle);
  Serial.print(" ");
  Serial.print(regen);
  Serial.print(" ");
  Serial.println(dir);
}

void msgHandleLightCtrl(byte rx_status, byte length, uint32_t frame_id, byte filter, byte buffer, byte *frame_data, byte ext){
  Serial.println(frame_data[0]);
}

void msgHandler(byte rx_status, byte length, uint32_t frame_id, byte filter, byte buffer, byte *frame_data, byte ext) {
   
   if(frame_id == CAN_ID_MOTOR_CTRL){
     msgHandleMotorCtrl(rx_status, length, frame_id, filter, buffer, frame_data, ext);
   }else if(frame_id == CAN_ID_SIGNAL_CTRL){
     msgHandleLightCtrl(rx_status, length, frame_id, filter, buffer, frame_data, ext);
   }else{
     Serial.print("unknown msg ");
     printBuf(rx_status, length, frame_id, filter, buffer, frame_data, ext); 
   }
}

void loop() {
  
  byte length,rx_status,filter,ext;
  uint32_t frame_id;
  byte frame_data[8];
  
  rx_status = CAN.readStatus();

  if ((rx_status & 0x40) == 0x40) {

    CAN.readDATA_ff_0(&length,frame_data,&frame_id, &ext, &filter);
    msgHandler(rx_status, length, frame_id, filter, 0, frame_data, ext);
    CAN.clearRX0Status();
    rx_status = CAN.readStatus();
  }
      
  if ((rx_status & 0x80) == 0x80) {

    CAN.readDATA_ff_1(&length,frame_data,&frame_id, &ext, &filter);
    msgHandler(rx_status, length, frame_id, filter, 1, frame_data, ext);
    CAN.clearRX1Status();
    rx_status = CAN.readStatus();
  }
}

