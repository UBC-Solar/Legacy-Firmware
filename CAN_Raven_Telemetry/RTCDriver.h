#ifndef TELEMETRY_RTCDRIVER_H
#define TELEMETRY_RTCDRIVER_H

#include "DataTypes.h"

/**
 * @brief Driver for DS1302 real time clock.
 */
class RTCDriver {
 public:
  /**
   * Create a new RTCDriver object that controls and gets the time from the
   * DS1302 chip.
   */
  RTCDriver(uint8_t pin=6, uint32_t maxClock=1000000);

  /**
   * Initalize the chip on a pre-specific pin.
   */
  void begin();

  /**
   * Get the current date and time.
   * @return A |DateTime| struct containing the current date and time.
   */
  struct DateTime getTime();

  /**
   * Write the current date and time to pre-existing DateTime struct.
   * @param dateTime Pointer to the struct.
   */
  void getTime(struct DateTime* dateTime);

  /**
   * Check that the DateTime struct represents a valid date and time.
   * @param dateTime Pointer to the struct.
   */
  bool isValid(struct DateTime* dateTime);

 private:
  uint8_t _pin;
  uint32_t _maxClock;

  // Get the expected number of days for the month
  uint8_t daysPerMonth(uint8_t month, uint8_t year);
}

#endif // TELEMETRY_RTCDRIVER_H
