//ZEVA PROTOCOL AT http://zeva.com.au/Products/datasheets/BMS12_CAN_Protocol.pdf
#include "CAN_Raven_Motor.h"
#include <SoftwareSerial.h>
#include <SPI.h>
#include <ubcsolar_can_ids.h>

void setup() {
  Serial.begin(115200);
  lastHeartbeatMillis = millis();
  canDriver.begin(); // also inits SPI, we believe

/* RHEO INIT */
  pinMode(RHEO_THROTTLE_SS,OUTPUT);
  pinMode(RHEO_REGEN_SS,OUTPUT);
  digitalWrite(RHEO_THROTTLE_SS, HIGH);
  digitalWrite(RHEO_REGEN_SS, HIGH);
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
}

// Does the rheo actually require us to use the 2-byte spi transfers?
void setRheo(uint8_t controlPin, uint8_t value) {
  digitalWrite(controlPin, LOW);
  SPI.transfer(0x00);
  SPI.transfer(value & 0xFF);
  digitalWrite(controlPin, HIGH);
  delayMicroseconds(1);
}

void motorSwitchDir(uint8_t dir) {
  // this function should only be called when the direction changes
  digitalWrite(FORWARD_SWITCH, LOW);
  digitalWrite(REVERSE_SWITCH, LOW);
  delay(100);
  if (dir == DIRECTION_FORWARD) {
    digitalWrite(FORWARD_SWITCH, HIGH);
  } else if (dir == DIRECTION_REVERSE) {
    digitalWrite(REVERSE_SWITCH, HIGH);
  }
}

void motorCtrlRun() {

  if (current_dir != target_dir) {
    transitioning_direction = true;
    dir_switch_counter = millis();
  }
  if (transitioning_direction) {
    if (millis() - dir_switch_counter > DIR_SWITCH_MS){
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
          transitioning_direction = 0;
        }
      }
      dir_switch_counter += DIR_SWITCH_MS;
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

void loop() {
  Message message;
  HeartbeatMessage* heartbeat;
  static uint32_t millisSinceHeartbeat;
  static const uint32_t cutoffPeriod = 2*HeartbeatMessage::HEARTBEAT_PERIOD;
  millisSinceHeartbeat += millis() - lastHeartbeatMillis;
  lastHeartbeatMillis = millis();

  if (canDriver.checkMessage(message)) {
    switch (message._frameId) {
      case CAN_ID_HANDBRAKE:
        // turn off the motor
        target_throttle = 0;
        break;
      case CAN_ID_HEARTBEAT:
        // reset timer
        millisSinceHeartbeat = 0;
        heartbeat = static_cast<HeartbeatMessage*>(&message);
        // set target values according to message contents
        target_throttle = heartbeat->getForwardThrottle();
        target_regen = heartbeat->getRegen();
        target_dir = heartbeat->getThrottleDirection();
        break;
      default:
        // ignore it, we don't care about other messages
        break;
    }
  }

  if (millisSinceHeartbeat > cutoffPeriod) {
    // panic, send error message, we've missed at least one heartbeat
    CommErrorMessage msg(CommErrorMessage::SYSTEM_MOTOR_CONTROL, millisSinceHeartbeat);
    canDriver.send(msg);
    // also turn off the motor
    target_throttle = 0;
  }

  motorCtrlRun();
}
