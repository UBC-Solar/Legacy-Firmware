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

#define CAN_ID_SIGNAL_CTRL 9
#define CAN_ID_MOTOR_CTRL 4
#define CAN_ID_MPPT_CURRENT 200
#define CAN_ID_UI_BUTTONS 700

#define BIT_SIGNAL_BRAKE 1
#define BIT_SIGNAL_LEFT 2
#define BIT_SIGNAL_RIGHT 4

#define MOTOR_CTRL_INTERVAL 20
#define SIGNAL_CTRL_INTERVAL 500
#define MPPT_CURRENT_INTERVAL 1000

#define MPPT_SENSOR_PIN A0
#define MPPT_SENSOR_SHUNT_R 0.002
#define MPPT_SENSOR_AMP_GAIN 20

byte length,rx_status,filter,ext;
uint32_t frame_id;
byte frame_data[8];

union floatbytes {
  byte b[4];
  float f;
} mppt_i;

byte motor_acceleration;
byte motor_regen;
byte motor_direction;

byte signal_lights;

unsigned long int lastTX = millis();

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

void sendMPPTCurrentPacket(void){     
  frame_id = CAN_ID_MPPT_CURRENT;
  length = 4;
  
  frame_data[0] = mppt_i.b[0];
  frame_data[1] = mppt_i.b[1];
  frame_data[2] = mppt_i.b[2];
  frame_data[3] = mppt_i.b[3];
  
  while(CAN.TX1Busy());
  CAN.load_ff_1(length,&frame_id,frame_data, false);
}

void sendMotorControlPacket(void){
  frame_id = CAN_ID_MOTOR_CTRL;
  length = 3;
  
  frame_data[0] = motor_acceleration;
  frame_data[1] = motor_regen;
  frame_data[2] = motor_direction;
  
  while(CAN.TX0Busy());
  CAN.load_ff_0(length,&frame_id,frame_data, false);
}

void sendSignalControlPacket(void){
  frame_id = CAN_ID_SIGNAL_CTRL;
  length = 1;
  
  frame_data[0] = signal_lights;
  
  while(CAN.TX1Busy());
  CAN.load_ff_1(length,&frame_id,frame_data, false);
}

void MPPTCurrent(){
  static unsigned long int lastTX = millis();
  if(millis() - lastTX + 250 > MPPT_CURRENT_INTERVAL){
    mppt_i.f = (analogRead(MPPT_SENSOR_PIN) - 511.5) * 5.0 / 1024 / MPPT_SENSOR_AMP_GAIN / MPPT_SENSOR_SHUNT_R;
    sendMPPTCurrentPacket();
    lastTX += MPPT_CURRENT_INTERVAL;
    //Serial.print("M");
  }
}

void motorControl(){
  static unsigned long int lastTX = millis();
  if(millis() - lastTX > MOTOR_CTRL_INTERVAL){
    sendMotorControlPacket();
    lastTX += MOTOR_CTRL_INTERVAL;
    Serial.print("m");
  }
}

void signalControl(){
  static unsigned long int lastTX = millis();
  if(millis() - lastTX > SIGNAL_CTRL_INTERVAL){
    signal_lights ^= (BIT_SIGNAL_LEFT + BIT_SIGNAL_RIGHT);
    sendSignalControlPacket();
    lastTX += SIGNAL_CTRL_INTERVAL;
    //Serial.print("s");
  }
}

void loop() {  
  MPPTCurrent();
  motorControl();
  signalControl();
  if(millis() - lastTX > 1000){
    lastTX += 1000;
    Serial.println();
  }
}

