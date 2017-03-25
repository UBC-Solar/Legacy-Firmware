#include "RTCDriver.h"

RTCDriver::RTCDriver(uint8_t pin, uint32_t maxClock)
  : _pin(pin), _maxClock(maxClock) {}

void RTCDriver::begin() {
  // TODO: check out DS1302 library, may make this more straightforward
  // http://playground.arduino.cc/Main/DS1302RTC
}

DateTime RTCDriver::getTime() {
  // stub -- returns empty DateTime for now
  DateTime dateTime(0, 0, 0, 0, 0, 0);
  return dateTime;
}

void RTCDriver::getTime(DateTime* dateTime) {
  // nothing for now
}
