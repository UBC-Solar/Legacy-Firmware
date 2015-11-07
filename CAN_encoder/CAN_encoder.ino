/*  New encoder written to support pseudo multitasking
    To ensure accurate encoder readings, the other tasks should exit quickly and return control to the encoder 
    so the encoder can detect the high and low of each cycle.
*/

#include <CAN.h>
#include <SoftwareSerial.h>
#include <SPI.h>

#define BUS_SPEED 125
#define CAN_ID_SPEED_SENSOR 20

#define ENC_PIN 4  // analog pin for reading the IR sensor
#define ENC_TIMEOUT 5000000 //microseconds
#define ENC_AVERAGE_PERIOD 5000000.0 //microseconds, make sure to add .0 to the end to force floating point or it will fail
#define ENC_AVERAGE_MIN_NEW_VALUE_WEIGHT 500000 //microseconds

byte length,rx_status,filter,ext;
uint32_t frame_id;
byte frame_data[8];

union floatbytes {
  byte b[4];
  float f;
} freq;

void setup() {
  Serial.begin(115200);     // start the serial port
  pinMode(ENC_PIN, INPUT);
  Serial.flush();
  
  CAN.begin();
  CAN.setMode(CONFIGURATION);
  CAN.baudConfig(BUS_SPEED);
  CAN.setMode(NORMAL);  // set to "NORMAL" for standard com
  CAN.toggleRxBuffer0Acceptance(true, true);
  CAN.toggleRxBuffer1Acceptance(true, true); 
  CAN.resetFiltersAndMasks();

  length = 4;
  frame_id = CAN_ID_SPEED_SENSOR;
}

void loop(){  
  encoder();
}

void encoder(){
  static unsigned long lastTime=micros();
  static unsigned long thisTime;
  static unsigned long period;
  static boolean lastState=0;
  static boolean timedout=false;
  static boolean firstTime=true;
  
  //enable these for serial request/response protocol
//  if(Serial.available() && Serial.read()=='s')
//    Serial.println(freq.f);
  
  thisTime = micros();
  if(thisTime-lastTime > ENC_TIMEOUT){
    freq.f=0;
    timedout=true;
    lastTime=thisTime;
    //enable these for automatic sending (good for CAN)
    Serial.println(freq.f);
    CAN.load_ff_0(length,&frame_id, freq.b, false); //send the bytes directly, don't copy
  }else{
    if(lastState){
      if(!digitalRead(ENC_PIN)){ //detect falling edge
        if(timedout){
          lastTime=thisTime;
          timedout=false;
          lastState=false;
          firstTime=true;
        }else{
          period=thisTime-lastTime;
          if(firstTime || period>ENC_AVERAGE_PERIOD){
            freq.f=1000000.0/period; //instantaneous speed
            firstTime=false;
          }else{
            unsigned long periodForAvg = period > ENC_AVERAGE_MIN_NEW_VALUE_WEIGHT ? period : ENC_AVERAGE_MIN_NEW_VALUE_WEIGHT;
            freq.f=freq.f * (1-periodForAvg/ENC_AVERAGE_PERIOD) + (1000000.0/period) /*new speed*/ * (periodForAvg/ENC_AVERAGE_PERIOD); //average speed readings to reduce fluctuations
          }
          lastTime=thisTime;
          lastState=false;
          //enable these for automatic sending (good for CAN)
          Serial.println(freq.f);
          CAN.load_ff_0(length,&frame_id, freq.b, false); //send the bytes directly, don't copy
        }
      }
    }else{
      lastState=digitalRead(ENC_PIN);
    }
  }
}
