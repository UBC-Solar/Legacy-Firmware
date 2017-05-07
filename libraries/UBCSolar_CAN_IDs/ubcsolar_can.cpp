#include "ubcsolar_can.h"


uint8_t HeartbeatMessage::getForwardThrottle() {
  return _buffer[FORWARD_THROTTLE_INDEX];
}

uint8_t HeartbeatMessage::getRegen() {
  return _buffer[REVERSE_THROTTLE_INDEX];
}

HeartbeatMessage::Direction HeartbeatMessage::getThrottleDirection() {
  return Direction(_buffer[DIRECTION_STATUS_INDEX]);
}

HeartbeatMessage::Signal HeartbeatMessage::getSignals() {
  return Signal(_buffer[INTERFACE_STATUS_INDEX] & (RIGHT_SIGNAL_MASK | LEFT_SIGNAL_MASK));
}

CommErrorMessage::SystemId CommErrorMessage::getSystemId() {
  return SystemId(_buffer[SYSTEM_ID_INDEX]);
}

uint32_t CommErrorMessage::getTimestamp() {
  return _buffer[TIMESTAMP_INDEX];
}

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

  Serial.print("CAN initialized on pin ");
  Serial.println(activePin);
}

boolean CANDriver::checkMessage(Message& message) {
  if (_can.checkReceive() != CAN_MSGAVAIL) {
    return false;
  }
  uint8_t result = _can.readMsgBufID(&message._frameId, &message._msgLength, message._buffer);

  return result == CAN_OK;
}
