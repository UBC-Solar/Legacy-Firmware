#include <FlexCAN.h>
#include <kinetis_flexcan.h>
#include <ubcsolar_can_ids.h>

#define RELAY_PIN 10
#define TRANSMIT_INTERVAL 100

FlexCAN CANbus(125000);
static CAN_message_t rxmsg, txmsg;

bool fault = false;

void setup() {

  Serial.begin(115200);
  
  CANbus.begin();
  txmsg.id = CAN_ID_AUX_BMS;
  txmsg.len = 1;

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // power indicator
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
}

void loop() {

  if(CANbus.available()) {
    CANbus.read(rxmsg);

    Serial.print("received canbus message with id ");
    Serial.println(rxmsg.id);
    
    if(rxmsg.id == CAN_ID_ZEVA_BMS_CORE_STATUS) {
      
      // error code from ZEVA
      int e = rxmsg.buf[0] & 0b00001111;

      // overcurrent or undervoltage or overvoltage or overtemp
      if(e == 3 || e == 5 || e == 7 || e == 8 ) {
        digitalWrite(RELAY_PIN, LOW);
        fault = true;
        txmsg.buf[0] = 1;
      }
    }
    else if(rxmsg.id == CAN_ID_HEARTBEAT) {

    }
  }

  if(millis() % TRANSMIT_INTERVAL == 0) {
    CANbus.write(txmsg);
  }
}
