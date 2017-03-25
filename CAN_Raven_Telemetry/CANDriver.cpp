#include "CANDriver.h"

CANDriver::CANDriver(uint8_t pin, uint8_t offset, uint8_t baudrate)
  : _pin(pin), _offset(offset), _baudrate(baudrate) {
    MCP_CAN _can(pin);
}

void CANDriver::begin() {
  uint8_t activePin = _pin;
  while (CAN.begin(baudrate) != CAN_OK) {
    // Alternate between two possible pins until the correct pin is found
    activePin = (activePin == _pin) ? _alternate : _pin;
    delay(100);
    _can = MCP_CAN(activePin);
  }

  Serial.println(("CAN initialized on pin %d", activePin));
}

void CANDriver::getMessage(DataLogger* logger) {
  if (_can.checkReceive() == CAN_MSGAVAIL) {
    _can.readMsgBufID(&_frameId, &_msgLength, _message);
    logger->receiveData(_frameId, _message);
  }
}
