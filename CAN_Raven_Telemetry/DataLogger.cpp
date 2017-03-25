#include "DataLogger.h"

DataLogger::DataLogger(uint8t pin) : _pin(pin) {
  _speed = 0;
  _voltage = 0;
  _state_of_charge = 0;
  _bms_temperature = 0;
  _motor_temperature = 0;
}

void DataLogger::begin() {
  if (_sdCard.begin(_pin)) {
    // Get the date
    DateTime date = timer.getTime();

    // Open or create log file for writing/appending
    char name[20];
    snprintf(name, 20, "log_%d-%d-%d.csv", date.year, date.month, date.day);
    if (_logFile.open(fileName, O_RDWR | O_CREAT | O_AT_END) {
      Serial.println(("SD card initialized on pin %d", _pin));
    }
  }

  Serial.println(("Failed to initialize SD card on pin %d", _pin));
}

void DataLogger::receiveData(const uint32_t id, const uint8_t message) {
  switch (id) {
    case CAN_ID_BRAKE:
      // log brake
      break;
    case CAN_ID_HAZARD:
      // log hazard
      break;
    case CAN_ID_MOTOR_CTRL:
      // log ?
      break;
    case CAN_ID_SPEED_SENSOR:
      _speed = message;
      break;
    case CAN_ID_SIGNAL_CTRL:
      // log?
      break;
    // other cases here
    // may have to separate the BMS messages into their own handler, for
    // better readability
    default:
      // ?
  }
}

void DataLogger::log() {
  // write data to SD card
}

boolean DataLogger::shouldWrite() {
  // Logic to decide whether it is time to write.
  // Should always write when important things change, e.g. brake status,
  // hazard lights. Aside from that, how should it decide? We'll have to
  // talk this through at some point.
  // For now, just always return true
  return true;
}
