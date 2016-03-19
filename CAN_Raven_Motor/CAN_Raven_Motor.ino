//ZEVA PROTOCOL AT http://zeva.com.au/Products/datasheets/BMS12_CAN_Protocol.pdf

#include <mcp_can.h>
#include <SoftwareSerial.h>
#include <SPI.h>

#include <ubcsolar_can_ids.h>

#define CAN_SS 9
#define RHEO_THROTTLE_SS 8
#define RHEO_REGEN_SS 7

/* relays, from top to bottom */
#define RELAY1PIN 4
#define RELAY2PIN 5
#define RELAY3PIN 6
#define RELAY4PIN 3

#define FORWARD_SWITCH RELAY1PIN
#define REVERSE_SWITCH RELAY2PIN
#define SEAT_SWITCH RELAY3PIN
#define FS1_SWITCH RELAY4PIN

#define SPEED_SENSOR_PIN A0
#define SPEED_SENSOR_TIMEOUT 5000000 //microseconds
#define SPEED_SENSOR_AVG_PERIOD 5000000.0 //microseconds, make sure to add .0 to the end to force floating point or it will fail
#define SPEED_SENSOR_AVERAGE_MIN_NEW_VALUE_WEIGHT 500000 //microseconds

#define DIR_SWITCH_RAMP_STEP_TIME_MS 10


#define BUS_SPEED CAN_125KBPS

MCP_CAN CAN(CAN_SS);

byte target_throttle = 0;
byte target_regen = 0;
byte target_dir = 0;
byte current_throttle = 0;
byte current_regen = 0;
byte current_dir = 0;
byte in_dir_switch = 0;
byte dir_switch_last_ramp_time = 0;
byte brake_on = 0;
unsigned long int lastMotorCtrlRxTime = 0;

union floatbytes {
  byte b[4];
  float f;
} speedHz;


void setup() {  
/* SERIAL INIT */
  Serial.begin(115200);

/* CAN INIT */
  int canSSOffset = 0;

CAN_INIT:

  if(CAN_OK == CAN.begin(BUS_SPEED))                   // init can bus : baudrate = 125k
  {
    Serial.println("CAN BUS Shield init ok!");
  }
  else
  {
    Serial.println("CAN BUS Shield init fail");
    Serial.print("Init CAN BUS Shield again with SS pin ");
    Serial.println(CAN_SS + canSSOffset);
    delay(100);
    canSSOffset ^= 1;
    CAN = MCP_CAN(CAN_SS + canSSOffset);
    goto CAN_INIT;
  }

  CAN.init_Mask(0, 0, 0x3ff); //mask 0 (for buffer 0) controls filters 0 and 1
  CAN.init_Mask(1, 0, 0x3ff); //mask 1 (for buffer 1) controls filters 2 to 5

  //to pass a filter, all bits in the msg id that are "masked" must be the same as in the filter.
  //a message will pass if at least one of the filters pass it. 
  CAN.init_Filt(0, 0, CAN_ID_MOTOR_CTRL);

/* RHEO INIT */
  pinMode(RHEO_THROTTLE_SS,OUTPUT);
  pinMode(RHEO_REGEN_SS,OUTPUT);
  digitalWrite(RHEO_THROTTLE_SS, HIGH);
  digitalWrite(RHEO_REGEN_SS, HIGH);
  //SPI init already done in CAN INIT

  setRheo(RHEO_THROTTLE_SS, 0);
  setRheo(RHEO_REGEN_SS, 0); // this code will be replaced with power on default setting in rheo EEPROM.

/* RELAY INIT */
  pinMode(RELAY1PIN, OUTPUT);
  pinMode(RELAY2PIN, OUTPUT);
  pinMode(RELAY3PIN, OUTPUT);
  pinMode(RELAY4PIN, OUTPUT);
  digitalWrite(RELAY1PIN, LOW);
  digitalWrite(RELAY2PIN, LOW);
  digitalWrite(RELAY3PIN, LOW);
  digitalWrite(RELAY4PIN, LOW);

/* MOTOR CONTROLLER INIT */
  digitalWrite(SEAT_SWITCH, LOW);
  digitalWrite(FORWARD_SWITCH, LOW);
  digitalWrite(REVERSE_SWITCH, LOW);
  digitalWrite(FS1_SWITCH, LOW);
  delay(1000);
  Serial.println("seat on");
  digitalWrite(SEAT_SWITCH, HIGH);
  delay(1000);
  Serial.println("fwd on");
  digitalWrite(FORWARD_SWITCH, HIGH);
  delay(1000);
  Serial.println("fs1 on");
  digitalWrite(FS1_SWITCH, HIGH);
  delay(1000);

/* SPEED SENSOR INIT */
  pinMode(SPEED_SENSOR_PIN, INPUT);
}

void printBuf(uint32_t frame_id, byte *frame_data, byte length) {
     
  Serial.print("[Rx] ID: ");
  Serial.print(frame_id,HEX);
        
  Serial.print(" Len:");
  Serial.print(length,HEX);
      
  Serial.print(" Data:[");
  for (int i=0;i<length;i++) {
    if (i>0) Serial.print(" ");
    Serial.print(frame_data[i],HEX);
  }
  Serial.println("]"); 
}

void msgHandleMotorCtrl(uint32_t frame_id, byte *frame_data, byte length){
  target_throttle = frame_data[0];
  target_regen = frame_data[1];
  target_dir = frame_data[2];
  lastMotorCtrlRxTime = millis();
  Serial.print(frame_data[0]);
  Serial.print(" ");
  Serial.print(frame_data[1]);
  Serial.print(" ");
  Serial.println(frame_data[2]);
}

void msgHandleBrake(uint32_t frame_id, byte *frame_data, byte length){
  brake_on = frame_data[0];
}

void msgHandler(uint32_t frame_id, byte *frame_data, byte length) {
   
   if(frame_id == CAN_ID_MOTOR_CTRL){
     msgHandleMotorCtrl(frame_id, frame_data, length);
   }else if(frame_id == CAN_ID_BRAKE){
     msgHandleBrake(frame_id, frame_data, length);
   }else{
     Serial.print("unknown msg ");
     printBuf(frame_id, frame_data, length);
   }
}

void setRheo(int ss, byte r){
  digitalWrite(ss,LOW);
  SPI.transfer(0x00);
  SPI.transfer(r&0xff);
  digitalWrite(ss,HIGH);
  delayMicroseconds(1);
}

void motorSwitchDir(int dir){
  // this function should only be called when the direction changes
  // 0=FWD, 1=REV
  digitalWrite(FORWARD_SWITCH, LOW);
  digitalWrite(REVERSE_SWITCH, LOW);
  delay(100);
  if(dir == 0)
    digitalWrite(FORWARD_SWITCH, HIGH);
  else if(dir == 1)
    digitalWrite(REVERSE_SWITCH, HIGH);
}

void motorCtrlRun(){
  if(current_dir != target_dir){
    in_dir_switch = 1;
    dir_switch_last_ramp_time = millis();
  }
  if(in_dir_switch){ // in direction transition
    if(millis() - dir_switch_last_ramp_time > DIR_SWITCH_RAMP_STEP_TIME_MS){
      if(current_dir != target_dir){ // ramp down
        current_throttle--;
        setRheo(RHEO_THROTTLE_SS, current_throttle);
        if(current_throttle == 0){
          current_dir = target_dir;
          motorSwitchDir(current_dir);
        }
      }else{ // ramp up
        current_throttle++;
        setRheo(RHEO_THROTTLE_SS, current_throttle);
        if(current_throttle >= target_throttle){
          in_dir_switch = 0;
        }
      }
      dir_switch_last_ramp_time += DIR_SWITCH_RAMP_STEP_TIME_MS;
    }
  }else{
    current_throttle = target_throttle;
    if(brake_on)
      current_throttle = 0; 
    current_regen = target_regen;
    setRheo(RHEO_THROTTLE_SS, current_throttle);
    setRheo(RHEO_REGEN_SS, current_regen);
  }
}

/* this function detects falling edges */
void speedSensorRun(){
  static unsigned long lastTime=micros();
  static unsigned long thisTime;
  static unsigned long period;
  static boolean lastState=0;
  static boolean timedout=false;
  static boolean firstTime=true;

  thisTime = micros();
  if(thisTime-lastTime > SPEED_SENSOR_TIMEOUT){
    speedHz.f=0;
    timedout=true;
    lastTime=thisTime;
    //enable these for automatic sending (good for CAN)
    Serial.println(speedHz.f);
    sendSpeedSensorPacket();
  }else{
    if(lastState){
      if(!digitalRead(SPEED_SENSOR_PIN)){ //detect falling edge
        if(timedout){
          lastTime=thisTime;
          timedout=false;
          lastState=false;
          firstTime=true;
        }else{
          period=thisTime-lastTime;
          if(firstTime || period>SPEED_SENSOR_AVG_PERIOD){
            speedHz.f=1000000.0/period; //instantaneous speed
            firstTime=false;
          }else{
            unsigned long periodForAvg = period > SPEED_SENSOR_AVERAGE_MIN_NEW_VALUE_WEIGHT ? period : SPEED_SENSOR_AVERAGE_MIN_NEW_VALUE_WEIGHT;
            speedHz.f=speedHz.f * (1-periodForAvg/SPEED_SENSOR_AVG_PERIOD) + (1000000.0/period) * (period/SPEED_SENSOR_AVG_PERIOD); //average speed readings to reduce fluctuations
          }
          lastTime=thisTime;
          lastState=false;
          //enable these for automatic sending (good for CAN)
          Serial.println(speedHz.f);
          sendSpeedSensorPacket();
        }
      }
    }else{
      lastState=digitalRead(SPEED_SENSOR_PIN);
    }
  }
}

void sendSpeedSensorPacket(){
  uint32_t frame_id = CAN_ID_SPEED_SENSOR;
  byte frame_data[8];
  byte length = 4;
  
  frame_data[0] = speedHz.b[0];
  frame_data[1] = speedHz.b[1];
  frame_data[2] = speedHz.b[2];
  frame_data[3] = speedHz.b[3];
  
  CAN.sendMsgBuf(frame_id, 0, length, frame_data);
}

void loop() {
  
  byte length;
  uint32_t frame_id;
  byte frame_data[8];

  if(CAN_MSGAVAIL == CAN.checkReceive()){
    CAN.readMsgBuf(&length, frame_data);
    frame_id = CAN.getCanId();

    msgHandler(frame_id, frame_data, length);
  }

  // stop motor if the commander is lost
  if(millis() - lastMotorCtrlRxTime > 500){
    target_throttle = 0;
  }
  
  speedSensorRun();
  motorCtrlRun();
}

