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

#define BUS_SPEED 125000

void setup()
{
  Serial.begin(9600);
  
  // initialize CAN bus class
  // this class initializes SPI communications with MCP2515
  CAN.begin();
  CAN.setMode(CONFIGURATION);
  CAN.baudConfig(BUS_SPEED);
  CAN.setMode(NORMAL); // set to "NORMAL" for standard com
  CAN.toggleRxBuffer0Acceptance(true, true);
  CAN.toggleRxBuffer1Acceptance(true, true); 
  CAN.resetFiltersAndMasks();
}

byte inc = 0;

void printBuf(byte rx_status, byte length, uint32_t frame_id, byte filter, byte buffer, byte *frame_data, byte ext)
{       
      Serial.print("[Rx] Status:");
      Serial.print(rx_status, HEX);
      
      Serial.print(" Len:");
      Serial.print(length, HEX);
      
      Serial.print(" Frame:");
      Serial.print(frame_id, HEX);

      Serial.print(" EXT?:");
      Serial.print(ext==1, HEX);
       
      Serial.print(" Filter:");
      Serial.print(filter, HEX);

      Serial.print(" Buffer:");
      Serial.print(buffer, HEX);
      
      Serial.print(" Data:[");
      for (int i=0;i<8;i++)
      {
        if (i>0) Serial.print(" ");
        Serial.print(frame_data[i], HEX);
      }
      Serial.println("]"); 
}

void loop()
{  
  byte length,rx_status,filter,ext;
  uint32_t frame_id;
  byte frame_data[8];
  
  Serial.println("sending an ext packet.");
     
  frame_data[0] = 0x55;
  frame_data[1] = 0xA4;
  frame_data[2] = 0x04;
  frame_data[3] = 0xF4;
  frame_data[4] = 0x14;
  frame_data[5] = 0xC4;
  frame_data[6] = 0xFC;
  frame_data[7] = 0x2D + inc;
  
  frame_id = 0x03012002;
  length = 7;
      
  CAN.load_ff_0(length,&frame_id,frame_data, true);
  printBuf(rx_status, length, frame_id, filter, 0, frame_data, ext);
  delay(1000);      
  
  Serial.println("sending a standard packet.");
     
  frame_data[0] = 0x55;
  frame_data[1] = 0xA4;
  frame_data[2] = 0x04;
  frame_data[3] = 0xF4;
  frame_data[4] = 0x14;
  frame_data[5] = 0xC4;
  frame_data[6] = 0xFC;
  frame_data[7] = 0x1D+inc;
  
  frame_id = 0x03012001;
  length = 7;
       
  CAN.load_ff_0(length,&frame_id,frame_data, 0);
  printBuf(rx_status, length, frame_id, filter, 0, frame_data, ext);
  delay(1000);
  inc++;
}

