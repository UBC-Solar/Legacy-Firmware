#define ZEVA_BMS_BASE_ID 100

#define ZEVA_BMS_CORE_STATUS 10
#define ZEVA_BMS_CORE_CONFIG_WR1 12
#define ZEVA_BMS_CORE_CONFIG_RD1 13
#define ZEVA_BMS_CORE_CONFIG_WR2 14
#define ZEVA_BMS_CORE_CONFIG_RD2 15
#define ZEVA_BMS_CORE_CONFIG_WR3 16
#define ZEVA_BMS_CORE_CONFIG_RD3 17

struct BMSConfig {
  byte valid;
  
  byte pack_capacity;
  byte soc_warn_thresh;
  byte full_voltage; // x2 to get real value
  byte current_warn_thresh; // x10 to get real value
  byte overcurrent_thresh; // x10 to get real value
  byte overtemperature_thresh;
  byte min_aux_voltage;
  byte max_leakage;
  
  byte tacho_pulses_per_rev;
  byte fuel_gauge_full;
  byte fuel_gauge_empty;
  byte temp_gauge_hot;
  byte temp_gauge_cold;
  byte peukerts_exponent; // /10 to get real value
  byte enable_precharge;
  byte enable_contactor_aux_sw;
  
  byte bms_min_cell_voltage; // x0.05V to get real value
  byte bms_max_cell_voltage; // x0.05V to get real value
  byte bms_shunt_voltage; // x0.05V to get real value
  byte low_temperature_warn;
  byte high_temperature_warn;
};
