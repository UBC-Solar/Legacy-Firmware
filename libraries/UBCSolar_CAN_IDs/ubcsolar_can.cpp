#include "ubcsolar_can.h"

HeartbeatMessage::HeartbeatMessage(const uint8_t& forwardThrottle,
  const uint8_t& reverseThrottle, const uint8_t& throttleDirection,
  const uint8_t& signalValues) {

    _frameId = CAN_ID_HEARTBEAT;
    _buffer[FORWARD_THROTTLE_INDEX] = forwardThrottle;
    _buffer[REVERSE_THROTTLE_INDEX] = reverseThrottle;
    _buffer[DIRECTION_STATUS_INDEX] = throttleDirection;
    _buffer[INTERFACE_STATUS_INDEX] = signalValues;
    _msgLength = MESSAGE_LENGTH;
}

uint8_t HeartbeatMessage::getForwardThrottle() {
  return _buffer[FORWARD_THROTTLE_INDEX];
}

uint8_t HeartbeatMessage::getRegen() {
  return _buffer[REVERSE_THROTTLE_INDEX];
}

uint8_t HeartbeatMessage::getThrottleDirection() {
  return _buffer[DIRECTION_STATUS_INDEX];
}

uint8_t HeartbeatMessage::getSignals() {
  return _buffer[INTERFACE_STATUS_INDEX] & (RIGHT_SIGNAL_MASK | LEFT_SIGNAL_MASK);
}

CommErrorMessage::CommErrorMessage(const uint8_t& systemId, uint32_t millisSinceLast) {
  _frameId = CAN_ID_COMM_ERROR;
  _buffer[SYSTEM_ID_INDEX] = systemId;
  _buffer[TIMESTAMP_INDEX] = millisSinceLast;
  _msgLength = MESSAGE_LENGTH;
}

uint8_t CommErrorMessage::getSystemId() {
  return _buffer[SYSTEM_ID_INDEX];
}

uint32_t CommErrorMessage::getMillisSinceLast() {
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

void CANDriver::send(Message& message) {
  _can.sendMsgBuf(message._frameId, 0, message._msgLength, message._buffer);
}
