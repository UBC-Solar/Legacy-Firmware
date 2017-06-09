#include <FlexCAN.h>
#include <kinetis_flexcan.h>

#include <ubcsolar_can_ids.h>

//TODO: figure out what pins we're using
#define HORN_PIN 0
#define LEFT_SIGNAL_PIN 1
#define RIGHT_SIGNAL_PIN 2
#define BRAKE_PIN 5
#define THROTTLE_PIN 14
#define DIRECTION_PIN 7

#define BYTE_THROTTLE_ACCEL 0
#define BYTE_THROTTLE_REGEN 1
#define BYTE_THROTTLE_DIR 2
#define BYTE_SIGNAL_STATUS 3

#define THROTTLE_THRESHOLD 0
#define THROTTLE_MULTIPLIER 1

#define REGEN_THRESHOLD 0
#define REGEN_MULTIPLIER 1

#define HEARTBEAT_TIME 100 //in ms

unsigned long int baudRate = 125000;
FlexCAN CANbus(baudRate);
static CAN_message_t txmsg, rxmsg;

unsigned long int previousMillis;
bool jumpHeartbeat = false;
byte stmp[8] = {0,0,0,0,0,0,0,0};

void setup() {
  Serial.begin(9600);
  
  CANbus.begin();
  txmsg.id = CAN_ID_HEARTBEAT;
  txmsg.len = 8;

  previousMillis = millis();

  pinMode(THROTTLE_PIN, INPUT);
  
  Serial.println(F("System initialized"));

}

void msgHandler() {
  
}

void sendHeartbeatMessage() {
  Serial.println("sending message");
  CANbus.write(txmsg);
  for(int i = 0; i < 8; i++){
    bitClear(txmsg.buf[BYTE_SIGNAL_STATUS], i);
  }
}

void processSignals() {
  if(digitalRead(LEFT_SIGNAL_PIN)){
    bitSet(txmsg.buf[BYTE_SIGNAL_STATUS],0);
  }
  if(digitalRead(RIGHT_SIGNAL_PIN)){
    bitSet(txmsg.buf[BYTE_SIGNAL_STATUS],1);
  }
  if(analogRead(BRAKE_PIN) > REGEN_THRESHOLD){
    bitSet(txmsg.buf[BYTE_SIGNAL_STATUS],2);
  }
  if(digitalRead(HORN_PIN)){
    bitSet(txmsg.buf[BYTE_SIGNAL_STATUS],3);
  }
}

int processThrottle() {
  int rawThrottle = analogRead(THROTTLE_PIN);
  //Serial.println("throttle value is: ");
  //Serial.println(rawThrottle);
  if(rawThrottle > THROTTLE_THRESHOLD) {
    return THROTTLE_MULTIPLIER * rawThrottle;
  } else {
    return 0;
  }
}

int processRegen() {
  int rawRegen = analogRead(BRAKE_PIN);
  if(rawRegen > REGEN_THRESHOLD) {
    return REGEN_MULTIPLIER * rawRegen;
  } else {
    return 0;
  }
}

void loop() {
  
  //check input, TODO: verify with David how each of these sections work
  txmsg.buf[BYTE_THROTTLE_ACCEL] = processThrottle();
  txmsg.buf[BYTE_THROTTLE_DIR] = digitalRead(DIRECTION_PIN);
  txmsg.buf[BYTE_THROTTLE_REGEN] = processRegen();

  processSignals();
  
  if(CANbus.read(rxmsg)){
    msgHandler();
  }
  
  if(jumpHeartbeat || (millis() - previousMillis > HEARTBEAT_TIME)){
    sendHeartbeatMessage();
    previousMillis = millis();
    jumpHeartbeat = false;
  }
}
