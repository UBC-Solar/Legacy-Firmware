#include "CANDriver.h"

CANDriver::CANDriver(uint8_t pin, uint8_t alternate, uint8_t baudrate)
  : _pin(pin), _alternate(alternate), _baudrate(baudrate), _can(pin) { }

void CANDriver::begin() {
  uint8_t activePin = _pin;
  while (_can.begin(_baudrate) != CAN_OK) {
    // Alternate between two possible pins until the correct pin is found
    activePin = (activePin == _pin) ? _alternate : _pin;
    delay(100);
    _can = MCP_CAN(activePin);
  }

  Serial.println(("CAN initialized on pin %d", activePin));
}

void CANDriver::checkMessage(DataLogger* logger) {
  if (_can.checkReceive() == CAN_MSGAVAIL) {
    _can.readMsgBufID(&_frameId, &_msgLength, _message);
    logger->receiveData(_frameId, _message);
  }
}
