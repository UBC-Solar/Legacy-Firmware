#include "DataLogger.h"
#include <ubcsolar_can_ids.h>

DataLogger::DataLogger(uint8_t pin, RTCDriver* timer)
  : _pin(pin), _timer(timer) {
  _speed = 0;
  _voltage = 0;
  _state_of_charge = 0;
  _bms_temperature = 0;
  _motor_temperature = 0;
}

void DataLogger::begin() {
  if (_sdCard.begin(_pin, SPI_HALF_SPEED)) {
    // Get the date
    struct DateTime date = _timer->getTime();

    // Open or create log file for writing/appending
    char fileName[20];
    snprintf(fileName, 20, "log_%d-%d-%d.csv", date.year, date.month, date.day);
    if (_logFile.open(fileName, O_RDWR | O_CREAT | O_AT_END)) {
      Serial.print("SD card initialized on pin ");
      Serial.println(_pin);

      // print test line to the file
      DataLogger::write("File was opened for writing.");
      return;
    }
  }
  
  Serial.println("Failed to initialize SD card");
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
      return;
  }
}

void DataLogger::log() {
  if (!_logFile.isOpen()) {
    Serial.println("Failed to write to SD.");
  }

  struct DateTime date = _timer->getTime();

  // for now, just write the time
  char timeStamp[20];
  snprintf(timeStamp, 20, "%d:%d:%d", date.hour, date.minute, date.second);
  _logFile.println(timeStamp);
}

void DataLogger::write(const String msg) {
  if (!_logFile.isOpen()) {
    Serial.println("Failed to write to SD.");
    return;
  }
  
  _logFile.println(msg);
  _logFile.sync();
}

boolean DataLogger::shouldWrite() {
  // Logic to decide whether it is time to write.
  // Should always write when important things change, e.g. brake status,
  // hazard lights. Aside from that, how should it decide? We'll have to
  // talk this through at some point.
  // For now, just always return true
  return true;
}
