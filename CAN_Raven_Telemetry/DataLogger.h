#ifndef TELEMETRY_DATALOGGER_H
#define TELEMETRY_DATALOGGER_H

#include "RTCDriver.h"
#include <SdFat.h>

/**
 * @brief Holds and logs data.
 */
class DataLogger {
 public:
  /**
   * Create a new empty |DataLogger| that holds data and handles its own
   * logging logic.
   * @param pin The Arduino pin for reading/writing.
   * @param timer Point to the |RTCDriver| object that handles the timer.
   */
  DataLogger(uint8_t pin, RTCDriver* timer);

  /**
   * Initialize the DataLogger so that it can parse and log messages.
   */
  void begin();

  /**
   * Parse and log a CAN message.
   * @param id The CAN message ID.
   * @param message The CAN message.
   */
  void receiveData(const uint32_t id, const uint8_t message);

  /**
   * Log a datapoint to the SD card.
   */
  void log();

  /**
   * Write a message to the SD card.
   */
  void write(const String msg);

 private:
  // SD card
  uint8_t _pin;
  SdFat _sdCard;
  SdFile _logFile;

  // Timer
  RTCDriver* _timer;

  // Data fields
  uint8_t _speed;
  uint8_t _voltage;
  uint8_t _state_of_charge;
  uint8_t _bms_temperature;
  uint8_t _motor_temperature;
  uint8_t _battery_temperature[4];
  uint8_t _cell_voltage_x50[4][12];

  // Helper function to decide whether it's time to write to log.
  boolean shouldWrite();
};

#endif // TELEMETRY_DATALOGGER_H
