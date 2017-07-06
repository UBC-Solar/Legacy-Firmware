#include <FlexCAN.h>
#include <kinetis_flexcan.h>
#include <ubcsolar_can_ids.h>

#define RELAY_PIN 10

FlexCAN CANbus(125000);
static CAN_message_t rxmsg;

void setup() {

  Serial.begin(115200);
  CANbus.begin();

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
}

void loop() {

  if(CANbus.available()) {
    CANbus.read(rxmsg);
    
    if(rxmsg.id == CAN_ID_ZEVA_BMS_CORE_STATUS) {
      int error = rxmsg.buf[0] & 0b00001111;

      if(error == 7 || error == 8) {
        digitalWrite(RELAY_PIN, LOW);
      }
      else {
        digitalWrite(RELAY_PIN, HIGH);
      }
    }
  }
}
