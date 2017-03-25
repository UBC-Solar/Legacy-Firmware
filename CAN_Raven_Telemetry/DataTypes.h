#ifndef TELEMETRY_DATATYPES_H
#define TELEMETRY_DATATYPES_H

/**
 * Represents an instant in time, to the nearest second
 */
struct DateTime {
  /// The 2-digit year
  uint8_t year;

  /// The month, represented as a number between 1 and 12, where 1 is January
  uint8_t month;

  /// The day of the month
  uint8_t day;

  /// The hour between 0 and 23
  uint8_t hour;

  /// The minute
  uint8_t minute;

  /// The second
  uint8_t second;
};

/**
 * Setup for the battery monitoring system
 */
struct BMSConfig {
  uint8_t valid;

  uint8_t pack_capacity;
  uint8_t soc_warn_thresh;
  uint8_t full_voltage; // x2 to get real value
  uint8_t current_warn_thresh; // x10 to get real value
  uint8_t overcurrent_thresh; // x10 to get real value
  uint8_t overtemperature_thresh;
  uint8_t min_aux_voltage;
  uint8_t max_leakage;

  uint8_t tacho_pulses_per_rev;
  uint8_t fuel_gauge_full;
  uint8_t fuel_gauge_empty;
  uint8_t temp_gauge_hot;
  uint8_t temp_gauge_cold;
  uint8_t peukerts_exponent; // /10 to get real value
  uint8_t enable_precharge;
  uint8_t enable_contactor_aux_sw;

  uint8_t bms_min_cell_voltage; // x0.05V to get real value
  uint8_t bms_max_cell_voltage; // x0.05V to get real value
  uint8_t bms_shunt_voltage; // x0.05V to get real value
  uint8_t low_temperature_warn;
  uint8_t high_temperature_warn;
};

/**
 * The status of the battery management system
 */
struct BMSStatus {
  uint8_t status;
  uint8_t error;
  int soc;
  int voltage;
  int current;
  float aux_voltage;
  int temperature;
};

#endif // TELEMETRY_DATATYPES_H
