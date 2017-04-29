#include <avr/pgmspace.h>
#include "CANDriver.h"

#define SERIAL_BAUDRATE 19200
#define SDCARD_PIN 4

CANDriver driver = CANDriver();
RTCDriver timer = RTCDriver();
DataLogger logger = DataLogger(SDCARD_PIN, &timer);

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  // Serial.println(F("Telemetry node test"));

  // start CAN first
  // driver.begin();

  // start timer next
  timer.begin();
  Serial.println(F("Testing timer"));

  // start logger last
  // logger.begin();
}

void loop() {
  // driver.checkMessage(&logger);

  // test print functionality
  // logger.write("Look I wrote something.");

  // test timer
  struct DateTime dateTime = {0, 0, 0, 0, 0, 0};
  timer.getTime(&dateTime);
  char timeStamp[20];
  snprintf(timeStamp, 20, "%d:%d:%d", dateTime.hour, dateTime.minute, dateTime.second);
  Serial.print(F("The current time is "));
  Serial.println(timeStamp);
  
  delay(1000);
}
