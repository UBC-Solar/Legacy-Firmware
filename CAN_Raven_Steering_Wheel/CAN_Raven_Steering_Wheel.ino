#include <FlexCAN.h>
#include <kinetis_flexcan.h>
#include <ubcsolar_can_ids.h>

#define HORN_PIN 14
#define LEFT_SIGNAL_PIN 17
#define RIGHT_SIGNAL_PIN 15
#define REGEN_PIN 23
#define THROTTLE_PIN 20
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

#define HEARTBEAT_TIME 100 //in ms

int rawRegenRangeMin = 857;
int rawRegenRangeMax = 898;
int rawRegenFullRange = rawRegenRangeMax - rawRegenRangeMin;

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
  CANbus.write(txmsg);
  for(int i = 0; i < 8; i++){
    bitClear(txmsg.buf[BYTE_SIGNAL_STATUS], i);
  }
}

unsigned long lastLeftSignalDebounceTime = 0;
unsigned long lastRightSignalDebounceTime = 0;
unsigned long lastHornDebounceTime = 0;
unsigned long debounceDelay = 50;
int lastLeftSignalState = HIGH;
int lastRightSignalState = HIGH;
int lastHornState = HIGH;
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

    if(currentLeftSignalState == LOW) {
      inLeftSignalButtonPress = true;
    }
    else if(inLeftSignalButtonPress) {
      leftSignalOn = !leftSignalOn;
      inLeftSignalButtonPress = false;
    }
  }

  if(millis() - lastRightSignalDebounceTime > debounceDelay) {

    if(currentRightSignalState == LOW) {
      inRightSignalButtonPress = true;
    }
    else if(inRightSignalButtonPress) {
      rightSignalOn = !rightSignalOn;
      inRightSignalButtonPress = false;
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
  if(rawRegen > rawRegenRangeMin) {
    bitSet(txmsg.buf[BYTE_SIGNAL_STATUS],2);
  }

  int currentHornState = digitalRead(HORN_PIN);

  if(currentHornState != lastHornState) {
    lastHornDebounceTime = millis();
  }

  lastHornState = currentHornState;

  if(millis() - lastHornDebounceTime > debounceDelay) {
    if(currentHornState == LOW) {
      bitSet(txmsg.buf[BYTE_SIGNAL_STATUS], 3);
    }
  }
}

int rawThrottleRangeMin = 543;
int rawThrottleRangeMax = 610;
int rawThrottleFullRange = rawThrottleRangeMax - rawThrottleRangeMin;

byte processThrottle() {
  
  int rawThrottle = 0;
  for(int i=0; i<3; i++) {
    rawThrottle += analogRead(THROTTLE_PIN);
  }
  rawThrottle = rawThrottle / 3.0;

  // throttle pot decreases as trigger is pressed
  rawThrottle = 1023 - rawThrottle;

  if(rawThrottle < rawThrottleRangeMin) {
    rawThrottle = rawThrottleRangeMin;
  }
  else if(rawThrottle > rawThrottleRangeMax) {
    rawThrottle = rawThrottleRangeMax;
  }

  byte byteThrottle = float(rawThrottle - rawThrottleRangeMin) / rawThrottleFullRange * 255.0;
  return byteThrottle;
}

byte processRegen() {
  
  int rawRegen = 0;
  for(int i=0; i<3; i++) {
    rawRegen += analogRead(REGEN_PIN);
  }
  rawRegen = rawRegen / 3.0;

  if(rawRegen < rawRegenRangeMin) {
    rawRegen = rawRegenRangeMin;
  }
  else if(rawRegen > rawRegenRangeMax) {
    rawRegen = rawRegenRangeMax;
  }
  
  byte byteRegen = float(rawRegen - rawRegenRangeMin) / rawRegenFullRange * 255.0;
  return byteRegen;
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
