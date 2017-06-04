/*struct BMSConfig {
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
};*/
#define IDLE 0
#define PRECHARGING 1
#define RUNNING 2
#define CHARGING 3
#define SETUP 4

#define NOERR 0
#define CORRUPT 1
#define OVERCURRENTWARNING 2
#define OVERCURRENTSD 3
#define LOWCELLWARNING 4 
#define BMSSD 5
#define HIGHCELLWARNING 6
#define BMSENDEDCHARGE 7 
#define BMSOVERTEMP 8
#define LOWSOCWARNING 9
#define OVERTEMPERATURE 10
#define CHASISLEAK 11
#define LOW12V 12 
#define PRECHARGEFAIL 13
#define CONTRATORSWITCHERROR 14
#define CANERROR 15

struct BMSCoreStatus {
  byte status;
  byte error;
  int soc;
  int voltage;
  int current;
  float aux_voltage;
  int temperature;
};

struct Battery {
  uint32_t volt_warn;
  uint16_t volt_shun_warn;
  byte temp_warn;
  
  unsigned int cellVolts[12];
  byte temp[2];
};

struct Motor {
  byte target_throttle;
  byte target_regen;
  byte target_dir;
};

