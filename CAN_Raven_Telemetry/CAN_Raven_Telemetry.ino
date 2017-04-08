#include <SPI.h>
#include <avr/pgmspace.h>
#include "CANDriver.h"

#define SERIAL_BAUDRATE 19200

CANDriver driver = CANDriver();
RTCDriver timer = RTCDriver();
DataLogger logger = DataLogger(4, &timer);

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  Serial.println(F("Telemetry node test"));

  // start CAN first
  driver.begin();

  // start SPI second
  SPI.begin();

  // start timer third
  timer.begin();

  // start logger last
  logger.begin();
}

void loop() {
  driver.checkMessage(&logger);

  // decide when a datapacket should get logged
}
