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

#include <CAN.h>
#include <SoftwareSerial.h>
#include <SPI.h>

#define BUS_SPEED 125
#define ZEVA_ID 100

byte length,rx_status,filter,ext;
uint32_t frame_id;
byte frame_data[8];

void setup() {                

  
  Serial.begin(115200);
  
  // initialize CAN bus class
  // this class initializes SPI communications with MCP2515
  CAN.begin();
  
  CAN.setMode(CONFIGURATION);
  CAN.baudConfig(BUS_SPEED);
  
  /*CAN.setMaskOrFilter(MASK_0,   0b11111111, 0b11100000, 0b00000000, 0b00000000);
  CAN.setMaskOrFilter(FILTER_0, 0b11111111, 0b11100000, 0b00000000, 0b00000000); //DISALLOW ext packets.
  CAN.setMaskOrFilter(FILTER_1, 0b10111111, 0b11100000, 0b00000000, 0b00000000);
  CAN.setMaskOrFilter(MASK_1,   0b00000000, 0b00000000, 0b00001111, 0b11111111);  
  CAN.setMaskOrFilter(FILTER_2, 0b00000000, 0b00001000, 0b00001101, 0b11111111);
  CAN.setMaskOrFilter(FILTER_3, 0b00000000, 0b00001000, 0b00001101, 0b11111111);
  CAN.setMaskOrFilter(FILTER_4, 0b00000000, 0b00001000, 0b00001111, 0b11111111);
  CAN.setMaskOrFilter(FILTER_5, 0b00000000, 0b00001000, 0b00001101, 0b11111111);*/

 
  CAN.setMode(NORMAL);  // set to "NORMAL" for standard com

  CAN.toggleRxBuffer0Acceptance(true, true);
  CAN.toggleRxBuffer1Acceptance(true, true); 
  CAN.resetFiltersAndMasks();
}

void printBufTX(byte length, uint32_t frame_id, byte *frame_data, byte ext) {
      Serial.print("[TX]"); 
      
      Serial.print(" Len:");
      Serial.print(length,HEX);
      
      Serial.print(" Frame:");
      Serial.print(frame_id,HEX);

      Serial.print(" EXT?:");
      Serial.print(ext==1,HEX);
       
      Serial.print(" Data:[");
      for (int i=0;i<length;i++) {
        if (i>0) Serial.print(" ");
        Serial.print(frame_data[i],HEX);
      }
      Serial.println("]"); 
}

void fakeCurrentSensorPacket(void){
      static int test = 12345;
      union u_tag {
        byte b[4];
        float fval;
      } u;
      
      u.fval=test++;

      Serial.println(u.fval);
     
      frame_data[0] = u.b[0];
      frame_data[1] = u.b[1];
      frame_data[2] = u.b[2];
      frame_data[3] = u.b[3];
      frame_data[4] = 0x14;
      frame_data[5] = 0xC4;
      frame_data[6] = test;
      frame_data[7] = test>>8;
  
      frame_id = 0x7FF;
      length = 8;
}

void fakeBMSPacket(int bmsId, int voltGrp){
  frame_data[0]=1+20*bmsId+6*(voltGrp-1);
  frame_data[1]=2+20*bmsId+6*(voltGrp-1);
  frame_data[2]=3+20*bmsId+6*(voltGrp-1);
  frame_data[3]=4+20*bmsId+6*(voltGrp-1);
  frame_data[4]=5+20*bmsId+6*(voltGrp-1);
  frame_data[5]=6+20*bmsId+6*(voltGrp-1);
  frame_data[6]=0;
  frame_data[7]=77+128;
  
  frame_id=ZEVA_ID + 10*bmsId + 2*voltGrp + 1;
  length=8;
}

void loop() {
      fakeCurrentSensorPacket();
      printBufTX(length, frame_id, frame_data, false); //do not remove this print. the compiler will incorrectly optimize
      CAN.load_ff_0(length,&frame_id,frame_data, false);
      
      for(int i=0; i<4; i++){
        fakeBMSPacket(i,1);
        printBufTX(length, frame_id, frame_data, false); //do not remove this print. the compiler will incorrectly optimize
        CAN.load_ff_0(length,&frame_id,frame_data, false);
        fakeBMSPacket(i,2);
        printBufTX(length, frame_id, frame_data, false); //do not remove this print. the compiler will incorrectly optimize
        CAN.load_ff_0(length,&frame_id,frame_data, false);
      }
      
      //delay(1000);
}

