#ifndef CAN_RAVEN_MOTORH
#define CAN_RAVEN_MOTORH

#include <Arduino.h>
#include <stdint.h>
#include <ubcsolar_can.h>

#define CAN_SS_PIN 9
#define CAN_SS_PIN_ALT 10
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

#define DIR_SWITCH_MS 10

#define DIRECTION_FORWARD 0
#define DIRECTION_REVERSE 1

#define REGEN_MAX 255

// target system values
uint8_t target_throttle = 0;
uint8_t target_regen = 0;
uint8_t target_dir = 0;

// current system values
boolean transitioning_direction = false;
boolean brake_on = false;
uint8_t current_throttle = 0;
uint8_t current_regen = 0;
uint8_t current_dir = DIRECTION_FORWARD;
uint8_t dir_switch_counter = 0;

uint32_t lastHeartbeatMillis;

CANDriver canDriver(CAN_SS_PIN, CAN_SS_PIN_ALT);

void setRheo(uint8_t controlPin, uint8_t value);
void motorSwitchDir(uint8_t dir);
void motorCtrlRun();

#endif
