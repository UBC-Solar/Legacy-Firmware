#include <FlexCAN.h>
#include <kinetis_flexcan.h>
#include <ubcsolar_can_ids.h>

#define HORN_PIN 14
#define LEFT_SIGNAL_PIN 17
#define RIGHT_SIGNAL_PIN 15
#define REGEN_PIN 20
#define THROTTLE_PIN 23
#define DIRECTION_FWD_PIN 18
#define DIRECTION_REV_PIN 16

#define BMS_LAMP_RED_PIN 8
#define BMS_LAMP_GREEN_PIN 11
#define MOTOR_LAMP_RED_PIN 9
#define MOTOR_LAMP_GREEN_PIN 19
#define MPPT_LAMP_RED_PIN 2
#define MPPT_LAMP_GREEN_PIN 7
#define LOW_12V_LAMP_RED_PIN 6
#define LOW_12V_LAMP_GREEN_PIN 5
#define LEFT_TURN_LAMP_PIN 10
#define RIGHT_TURN_LAMP_PIN 12

#define BYTE_THROTTLE_ACCEL 0
#define BYTE_THROTTLE_REGEN 1
#define BYTE_THROTTLE_DIR 2
#define BYTE_SIGNAL_STATUS 3

#define THROTTLE_THRESHOLD 0
#define THROTTLE_MULTIPLIER 1 // should be <= 1

#define REGEN_THRESHOLD 0
#define REGEN_MULTIPLIER 1 // should be <= 1

#define HEARTBEAT_TIME 100 //in ms

unsigned long int baudRate = 125000;
FlexCAN CANbus(baudRate);
static CAN_message_t txmsg, rxmsg;

unsigned long int previousMillis;
unsigned long int previousSignalCheck;

bool jumpHeartbeat = false;
bool leftSignalOn = false;
bool rightSignalOn = false;

void setup() {
  Serial.begin(115200);
  
  CANbus.begin();
  txmsg.id = CAN_ID_HEARTBEAT;
  txmsg.len = 8;

  previousMillis = millis();

  pinMode(THROTTLE_PIN, INPUT);
  pinMode(REGEN_PIN, INPUT);
  pinMode(HORN_PIN, INPUT_PULLUP);
  pinMode(DIRECTION_FWD_PIN, INPUT_PULLUP);
  pinMode(DIRECTION_REV_PIN, INPUT_PULLUP);
  pinMode(LEFT_SIGNAL_PIN, INPUT_PULLUP);
  pinMode(RIGHT_SIGNAL_PIN, INPUT_PULLUP);
  
  pinMode(BMS_LAMP_RED_PIN, OUTPUT);
  pinMode(BMS_LAMP_GREEN_PIN, OUTPUT);
  pinMode(MOTOR_LAMP_RED_PIN, OUTPUT);
  pinMode(MOTOR_LAMP_GREEN_PIN, OUTPUT);
  pinMode(MPPT_LAMP_RED_PIN, OUTPUT);
  pinMode(MPPT_LAMP_GREEN_PIN, OUTPUT);
  pinMode(LOW_12V_LAMP_RED_PIN, OUTPUT);
  pinMode(LOW_12V_LAMP_GREEN_PIN, OUTPUT);
  pinMode(LEFT_TURN_LAMP_PIN, OUTPUT);
  pinMode(RIGHT_TURN_LAMP_PIN, OUTPUT);

  // flash the lamps so we can tell they're working
  for(int i=0; i<4; i++) {
    digitalWrite(BMS_LAMP_RED_PIN, HIGH);
    digitalWrite(MOTOR_LAMP_RED_PIN, HIGH);
    digitalWrite(MPPT_LAMP_RED_PIN, HIGH);
    digitalWrite(LOW_12V_LAMP_RED_PIN, HIGH);
    digitalWrite(LEFT_TURN_LAMP_PIN, HIGH);
    digitalWrite(RIGHT_TURN_LAMP_PIN, HIGH);
    delay(500);
    digitalWrite(BMS_LAMP_RED_PIN, LOW);
    digitalWrite(MOTOR_LAMP_RED_PIN, LOW);
    digitalWrite(MPPT_LAMP_RED_PIN, LOW);
    digitalWrite(LOW_12V_LAMP_RED_PIN, LOW);
    digitalWrite(LEFT_TURN_LAMP_PIN, LOW);
    digitalWrite(RIGHT_TURN_LAMP_PIN, LOW);
    digitalWrite(BMS_LAMP_GREEN_PIN, HIGH);
    digitalWrite(MOTOR_LAMP_GREEN_PIN, HIGH);
    digitalWrite(MPPT_LAMP_GREEN_PIN, HIGH);
    digitalWrite(LOW_12V_LAMP_GREEN_PIN, HIGH);
    delay(500);
    digitalWrite(BMS_LAMP_GREEN_PIN, LOW);
    digitalWrite(MOTOR_LAMP_GREEN_PIN, LOW);
    digitalWrite(MPPT_LAMP_GREEN_PIN, LOW);
    digitalWrite(LOW_12V_LAMP_GREEN_PIN, LOW);
  }
  
  Serial.println(F("System initialized"));
}

unsigned long lastZevaCoreMessageTime = 0;
unsigned long lastMotorControllerMessageTime = 0;
unsigned long lastMpptMessageTime = 0;
unsigned long thresholdTime = 250;
 
void processIndicatorLamps() {

  if(CANbus.available()) {
    CANbus.read(rxmsg);

    if(rxmsg.id == CAN_ID_ZEVA_BMS_CORE_STATUS) {
      lastZevaCoreMessageTime = millis();
      int error = rxmsg.buf[0] >> 4;

      if(error == 0) {
        // Zeva has reported no error
        digitalWrite(BMS_LAMP_GREEN_PIN, HIGH);
        digitalWrite(LOW_12V_LAMP_GREEN_PIN, HIGH);
        digitalWrite(BMS_LAMP_RED_PIN, LOW);
        digitalWrite(LOW_12V_LAMP_RED_PIN, LOW);
      }
      else if(error == 12) {
        // Zeva has reported 12V system voltage below warning level
        digitalWrite(BMS_LAMP_GREEN_PIN, HIGH);
        digitalWrite(LOW_12V_LAMP_GREEN_PIN, LOW);
        digitalWrite(BMS_LAMP_RED_PIN, LOW);
        digitalWrite(LOW_12V_LAMP_RED_PIN, HIGH);
      }
      else {
        // Zeva has reported some other error
        digitalWrite(BMS_LAMP_GREEN_PIN, LOW);
        digitalWrite(LOW_12V_LAMP_GREEN_PIN, HIGH);
        digitalWrite(BMS_LAMP_RED_PIN, HIGH);
        digitalWrite(LOW_12V_LAMP_RED_PIN, LOW);
      }
    }
  }

  if(millis() - lastZevaCoreMessageTime > thresholdTime) {
    digitalWrite(BMS_LAMP_GREEN_PIN, LOW);
    digitalWrite(LOW_12V_LAMP_GREEN_PIN, HIGH);
    digitalWrite(BMS_LAMP_RED_PIN, HIGH);
    digitalWrite(LOW_12V_LAMP_RED_PIN, LOW);
  }

  if(millis() - lastMotorControllerMessageTime > thresholdTime) {
    digitalWrite(MOTOR_LAMP_GREEN_PIN, LOW);
    digitalWrite(MOTOR_LAMP_RED_PIN, HIGH);
  }

  if(millis() - lastMpptMessageTime > thresholdTime) {
    digitalWrite(MPPT_LAMP_GREEN_PIN, LOW);
    digitalWrite(MPPT_LAMP_RED_PIN, HIGH);
  }
}

void sendHeartbeatMessage() {
  Serial.println("heartbeat");
  CANbus.write(txmsg);
  for(int i = 0; i < 8; i++){
    bitClear(txmsg.buf[BYTE_SIGNAL_STATUS], i);
  }
}

unsigned long lastLeftSignalDebounceTime = 0;
unsigned long lastRightSignalDebounceTime = 0;
unsigned long debounceDelay = 50;
int lastLeftSignalState = HIGH;
int lastRightSignalState = HIGH;
int debouncedLeftSignalState = HIGH;
int debouncedRightSignalState = HIGH;
bool inLeftSignalButtonPress = false;
bool inRightSignalButtonPress = false;

void processSignals() {

  int currentLeftSignalState = digitalRead(LEFT_SIGNAL_PIN);
  int currentRightSignalState = digitalRead(RIGHT_SIGNAL_PIN);

  if(currentLeftSignalState != lastLeftSignalState) {
    lastLeftSignalDebounceTime = millis();
  }
  if(currentRightSignalState != lastRightSignalState) {
    lastRightSignalDebounceTime = millis();
  }

  lastRightSignalState = currentRightSignalState;
  lastLeftSignalState = currentLeftSignalState;

  if(millis() - lastLeftSignalDebounceTime > debounceDelay) {
    if(debouncedLeftSignalState != currentLeftSignalState) {
      debouncedLeftSignalState = currentLeftSignalState;

      if(debouncedLeftSignalState == LOW) {
        inLeftSignalButtonPress = true;
      }
      else if(inLeftSignalButtonPress) {
        leftSignalOn = !leftSignalOn;
        inLeftSignalButtonPress = false;
      }
    }
  }

  if(millis() - lastRightSignalDebounceTime > debounceDelay) {
    if(debouncedRightSignalState != currentRightSignalState) {
      debouncedRightSignalState = currentRightSignalState;

      if(debouncedRightSignalState == LOW) {
        inRightSignalButtonPress = true;
      }
      else if(inRightSignalButtonPress) {
        rightSignalOn = !rightSignalOn;
        inRightSignalButtonPress = false;
      }
    }
  }

  if(rightSignalOn) {
    digitalWrite(RIGHT_TURN_LAMP_PIN, HIGH);
  }
  else {
    digitalWrite(RIGHT_TURN_LAMP_PIN, LOW);
  }

  if(leftSignalOn) {
    digitalWrite(LEFT_TURN_LAMP_PIN, HIGH);
  }
  else {
    digitalWrite(LEFT_TURN_LAMP_PIN, LOW);
  }
  
  if(leftSignalOn && rightSignalOn) {
    bitSet(txmsg.buf[BYTE_SIGNAL_STATUS], 4);
  }
  else if(leftSignalOn){
    bitSet(txmsg.buf[BYTE_SIGNAL_STATUS],0);
  }
  else if(rightSignalOn){
    bitSet(txmsg.buf[BYTE_SIGNAL_STATUS],1);
  }
  
  int rawRegen = analogRead(REGEN_PIN);
  byte byteRegen = rawRegen / 1023.0 * 255.0;
  if(byteRegen > REGEN_THRESHOLD) {
    bitSet(txmsg.buf[BYTE_SIGNAL_STATUS],2);
  }
  
  if(digitalRead(HORN_PIN)){
    bitSet(txmsg.buf[BYTE_SIGNAL_STATUS],3);
  }
}

byte processThrottle() {
  int rawThrottle = analogRead(THROTTLE_PIN);
  byte byteThrottle = rawThrottle / 1023.0 * 255.0;
  if(byteThrottle > THROTTLE_THRESHOLD) {
    return THROTTLE_MULTIPLIER * byteThrottle;
  } else {
    return 0;
  }
}

byte processRegen() {
  int rawRegen = analogRead(REGEN_PIN);
  byte byteRegen = rawRegen / 1023.0 * 255.0;
  if(byteRegen > REGEN_THRESHOLD) {
    return REGEN_MULTIPLIER * byteRegen;
  } else {
    return 0;
  }
}

byte processDirection() {

  if(digitalRead(DIRECTION_FWD_PIN) == LOW) {
    return 1;
  }
  else if(digitalRead(DIRECTION_REV_PIN) == LOW) {
    return 2;
  }

  return 0;
}

void loop() {
  
  txmsg.buf[BYTE_THROTTLE_ACCEL] = processThrottle();
  txmsg.buf[BYTE_THROTTLE_DIR] = processDirection();
  txmsg.buf[BYTE_THROTTLE_REGEN] = processRegen();

  processSignals();
  
  processIndicatorLamps();
  
  if(jumpHeartbeat || (millis() - previousMillis > HEARTBEAT_TIME)){
    sendHeartbeatMessage();
    previousMillis = millis();
    jumpHeartbeat = false;
  }
}
