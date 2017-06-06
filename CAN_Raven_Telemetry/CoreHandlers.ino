#include <ubcsolar_can_ids.h>
/*
  void zevaCoreStatusPrint(){
  Serial.print(F("Error: "));
  Serial.println(bmsStatus.error);
  Serial.print(F("Status: "));
  Serial.println(bmsStatus.status);
  Serial.print(F("State of Charge: "));
  Serial.println(bmsStatus.soc);
  Serial.print(F("Voltage: "));
  Serial.println(bmsStatus.voltage);
  Serial.print(F("Current: "));
  Serial.println(bmsStatus.current);
  Serial.print(F("Auxiliary Voltage: "));
  Serial.println(bmsStatus.aux_voltage);
  Serial.print(F("Temperature: "));
  Serial.println(bmsStatus.temperature);
  }

  void msgHandleZevaCoreStatus(uint32_t frame_id, byte *frame_data, byte length) {

  bmsStatus.status = frame_data[0]&15;
  bmsStatus.error = frame_data[0]>>4;
  bmsStatus.soc = frame_data[1];
  bmsStatus.voltage = (frame_data[2] + ((frame_data[3] & 0x0F) << 8));
  bmsStatus.current = ((frame_data[4] << 4) + ((frame_data[3] & 0xF0) >> 4)) - 2048;
  bmsStatus.aux_voltage = frame_data[5]/10.0;
  bmsStatus.temperature = frame_data[7];

  #if DEBUG
  Serial.println(F("BMS Core status packet"));
  zevaCoreStatusPrint();
  #endif

  bmsAlive |= 1;
  }

  void msgHandleZevaCoreConfigData1(uint32_t frame_id, byte *frame_data, byte length) {
  bmsConfig.pack_capacity = frame_data[0];
  bmsConfig.soc_warn_thresh = frame_data[1];
  bmsConfig.full_voltage = frame_data[2];
  bmsConfig.current_warn_thresh = frame_data[3];
  bmsConfig.overcurrent_thresh = frame_data[4];
  bmsConfig.overtemperature_thresh = frame_data[5];
  bmsConfig.min_aux_voltage = frame_data[6];
  bmsConfig.max_leakage = frame_data[7];
  bmsConfig.valid |= 1;

  Serial.print(F("Pack capacity [Ah]: "));
  Serial.println(bmsConfig.pack_capacity);

  Serial.print(F("SOC warning threshold [%]: "));
  Serial.println(bmsConfig.soc_warn_thresh);

  Serial.print(F("Full voltage [V]: "));
  Serial.println(2 * bmsConfig.full_voltage);

  Serial.print(F("Current warning threshold [A]: "));
  Serial.println(10 * bmsConfig.current_warn_thresh);

  Serial.print(F("Overcurrent threshold [A]: "));
  Serial.println(10 * bmsConfig.overcurrent_thresh);

  Serial.print(F("Over-temperature theshold [C]: "));
  Serial.println(bmsConfig.overtemperature_thresh);

  Serial.print(F("Minimum auxiliary voltage [V]: "));
  Serial.println(bmsConfig.min_aux_voltage);

  Serial.print(F("Maximum leakage [%]: "));
  Serial.println(bmsConfig.max_leakage);
  }

  void msgHandleZevaCoreConfigData2(uint32_t frame_id, byte *frame_data, byte length) {
  bmsConfig.tacho_pulses_per_rev = frame_data[0];
  bmsConfig.fuel_gauge_full = frame_data[1];
  bmsConfig.fuel_gauge_empty = frame_data[2];
  bmsConfig.temp_gauge_hot = frame_data[3];
  bmsConfig.temp_gauge_cold = frame_data[4];
  bmsConfig.peukerts_exponent = frame_data[5];
  bmsConfig.enable_precharge = frame_data[6];
  bmsConfig.enable_contactor_aux_sw = frame_data[7];
  bmsConfig.valid |= 2;

  Serial.print(F("Tacho pulses-per-rev (for gauge scaling): "));
  Serial.println(bmsConfig.tacho_pulses_per_rev);
  Serial.print(F("Fuel gauge full (for gauge scaling): "));
  Serial.println(bmsConfig.fuel_gauge_full);
  Serial.print(F("Fuel gauge empty: "));
  Serial.println(bmsConfig.fuel_gauge_empty);
  Serial.print(F("Temp gauge hot: "));
  Serial.println(bmsConfig.temp_gauge_hot);
  Serial.print(F("Temp gauge cold: "));
  Serial.println(bmsConfig.temp_gauge_cold);
  Serial.print(F("Peukerts exponent: "));
  Serial.println(bmsConfig.peukerts_exponent);
  Serial.print(F("Enable precharge: "));
  Serial.println(bmsConfig.enable_precharge);
  Serial.print(F("Enable contactor auxiliary switches: "));
  Serial.println(bmsConfig.enable_contactor_aux_sw);
  }

  void msgHandleZevaCoreConfigData3(uint32_t frame_id, byte *frame_data, byte length) {
  bmsConfig.bms_min_cell_voltage = frame_data[0];
  bmsConfig.bms_max_cell_voltage = frame_data[1];
  bmsConfig.bms_shunt_voltage = frame_data[2];
  bmsConfig.low_temperature_warn = frame_data[3];
  bmsConfig.high_temperature_warn = frame_data[4];
  bmsConfig.valid |= 4;

  Serial.print(F("BMS minimum cell voltage: "));
  Serial.println(bmsConfig.bms_min_cell_voltage);
  Serial.print(F("BMS maximum cell voltage: "));
  Serial.println(bmsConfig.bms_max_cell_voltage);
  Serial.print(F("BMS shunt voltage: "));
  Serial.println(bmsConfig.bms_shunt_voltage);
  Serial.print(F("Low temperature warning: "));
  Serial.println(bmsConfig.low_temperature_warn);
  Serial.print(F("High temperature warning: "));
  Serial.println(bmsConfig.high_temperature_warn);
  }


  void msgHandleZevaCoreConfig(uint32_t frame_id, byte *frame_data, byte length) {
  switch(frame_id){
    case CAN_ID_ZEVA_BMS_CORE_CONFIG_RD1:
      msgHandleZevaCoreConfigData1(frame_id, frame_data, length);
      break;
    case CAN_ID_ZEVA_BMS_CORE_CONFIG_RD2:
      msgHandleZevaCoreConfigData2(frame_id, frame_data, length);
      break;
    case CAN_ID_ZEVA_BMS_CORE_CONFIG_RD3:
      msgHandleZevaCoreConfigData3(frame_id, frame_data, length);
      break;
    default:
      break;
  }
  }

  void zevaCoreSetCellNum(void){
  byte length;
  uint32_t frame_id;
  byte frame_data[8];

  Serial.println(F("SEND CELL NUM"));
  frame_id = CAN_ID_ZEVA_BMS_CORE_SET_CELL_NUM;
  frame_data[0] = 0x0B;
  frame_data[1] = 0;
  frame_data[2] = 0;
  frame_data[3] = 0;
  frame_data[4] = 0;
  frame_data[5] = 0;
  frame_data[6] = 0;
  frame_data[7] = 0;
  length = 8;
  CAN.sendMsgBuf(frame_id, 0, length, frame_data);
  }*/

void msgHandleBrake(uint32_t frame_id, byte *frame_data, byte length) {
  brake_on = frame_data[0];

  printLogHeader(0);
  printHelper(F("Brake: "));

  printONOFF(brake_on, true);
}

void msgHandleHazard(uint32_t frame_id, byte* frame_data, byte length) {
  hazard = frame_data[0];

  printLogHeader(0);
  printHelper(F("Hazard: "));

  printONOFF(hazard, true);
}

void msgHandleMotor(uint32_t frame_id, byte* frame_data, byte length) {
  motor.target_throttle = frame_data[0];
  motor.target_regen = frame_data[1];
  motor.target_dir = frame_data[2];

  printLogHeader(0);
  printHelper(F("Throttle: "));
  printHelper(String(motor.target_throttle));
  printHelper(F("  Regen: "));
  printHelper(String(motor.target_regen));
  printHelper(F("  Direction: "));
  printHelper(String(motor.target_dir), NEW_LINE);
}

void msgHandleSpeed(uint32_t frame_id, byte* frame_data, byte length) {
  unsigned long temp = ((unsigned long) frame_data[0] << 24) | ((unsigned long) frame_data[1] << 16) | ((unsigned long) frame_data[2] << 8) | ((unsigned long) frame_data[3]);
  freq = *((float*) &temp);

  printLogHeader(0);
  printHelper(F("Frequency: "));

  printHelper(String(freq));
  printHelper(F(" rps"), NEW_LINE);
}

void msgHandleSignal(uint32_t frame_id, byte* frame_data, byte length) {
  left_signal = frame_data[0] & 0x1;
  right_signal = frame_data[0] & 0x2;

  printLogHeader(0);
  printHelper(F("Left signal: "));
  printONOFF(left_signal, false);
  printHelper(F("  Right signal: "));
  printONOFF(right_signal, true);
}

void msgHandleCoreStatus(uint32_t frame_id, byte* frame_data, byte length) {
  bms_status.status = frame_data[0] >> 4;
  bms_status.error = frame_data[0] & 0xF;
  bms_status.soc = frame_data[1];
  bms_status.voltage = ((unsigned long) frame_data[2]) | (((unsigned long) frame_data[3] & 0xF0) << 4);
  bms_status.current = (((unsigned long) frame_data[3] & 0xF) | ((unsigned long) frame_data[4] << 4)) - 2048;
  bms_status.aux_voltage = frame_data[5] / 10.0;
  bms_status.temperature = frame_data[7];

  if (!bms_status.error) {
    printLogHeader(0);
  }
  else if (bms_status.error == 2 || bms_status.error == 4 || bms_status.error == 6 || bms_status.error == 9) {
    printLogHeader(1);
  }
  else {
    printLogHeader(2);
  }
  printHelper(F("Status: "));

  printBMSCoreStatus();
  printHelper(F("   Error: "));
  printBMSCoreError();
  printHelper("", NEW_LINE);
}

void msgHandleBmsStatus(uint32_t frame_id, byte* frame_data, byte length) {
  int pack_num = (frame_id % 100) / 10;

  packs[pack_num].volt_warn = ((unsigned long) frame_data[2]) << 16 | ((unsigned long) frame_data[1]) << 8 | ((unsigned long) frame_data[0]);
  packs[pack_num].volt_shun_warn = frame_data[4] & 0x0F << 4 | (unsigned char) frame_data[3];
  packs[pack_num].temp_warn = frame_data[4] >> 4;
  printLogHeader((packs[pack_num].volt_warn | packs[pack_num].volt_shun_warn | packs[pack_num].temp_warn) == 0 ? 0 : 2);
  printHelper(F("Status of battery pack "));
  printHelper(String(pack_num) + ": ", NEW_LINE);
  Serial.println(packs[pack_num].volt_warn);

  for (int i = 0; i < 12; i++) {
    printHelper("Cell " + String(i) + ": ");
    printHelper("Voltage: ");
    printHelper((packs[pack_num].volt_warn & (0x01001 << i)) == 0 ? "OK" : (packs[pack_num].volt_warn & (0x01 << i)) == 0 ? "HIGH" : "LOW");
    printHelper("/");
    printHelper((packs[pack_num].volt_shun_warn & (0x01 << i)) == 0 ? "OK" : "SHUN", NEW_LINE);
  }

  for (int i = 0; i < 2; i++) {
    printHelper("Temperature " + String(i) + ": ");
    printHelper((packs[pack_num].temp_warn & (0x03 << i)) == 0 ? "OK" : packs[pack_num].temp_warn & (0x1 << i) == 0 ? "HIGH" : "LOW", NEW_LINE);
  }
}

void msgHandleBmsReply(uint32_t frame_id, byte* frame_data, byte length) {
  int pack_num = (frame_id % 100) / 10;
  int reply_type = frame_id % 10 == 3 ? 0 : 1;

  printLogHeader(0);
  printHelper(F("Battery pack "));

  printHelper(String(pack_num));
  printHelper(F(" info:"), NEW_LINE);

  for (int i = reply_type * 6; i < 6 * (1 + reply_type); i++) {
    packs[pack_num].cellVolts[i] = ((unsigned int) frame_data[i % 6]) | (frame_data[6] & ((unsigned int) 1 << i % 6)) << 8 - i % 6;

    printHelper(F("\t\t    Cell "));
    printHelper(String(i));
    printHelper(F(" Voltage: "));
    printHelper(String(packs[pack_num].cellVolts[i] / 100.0), NEW_LINE);
  }
  packs[pack_num].temp[reply_type] = frame_data[7] - 128;

  printHelper(F("\t\t    Temperature: "));
  printHelper(String(packs[pack_num].temp[reply_type]), NEW_LINE);
}

void msgHandleBms(uint32_t frame_id, byte* frame_data, byte length) {
  if ((frame_id % 10) % 2 == 0) {
    printLogHeader(3);
    printHelper(" Requesting ");
    printHelper((frame_id % 10 == 0) ? "Status" : "voltage " + String(frame_id % 10 / 2), NEW_LINE);
  }

  else if (frame_id % 10 == 1) {
    msgHandleBmsStatus(frame_id, frame_data, length);
  }

  else if ((frame_id % 10) == 3 || (frame_id % 10) == 5) {
    msgHandleBmsReply(frame_id, frame_data, length);
  }
}

void logToSD() {
  File logFile = SD.open(logFilename, FILE_WRITE);

  if (logFile) {
    logFile.print(myRTC.hours);
    logFile.print(F(":"));
    logFile.print(myRTC.minutes);
    logFile.print(F(":"));
    logFile.print(myRTC.seconds);
    logFile.print(F(","));
    logFile.print(F("PUT SPEED HERE"));//TODO: PUT SPEED HERE
    logFile.print(F(","));
    logFile.print(bms_status.soc);
    logFile.print(F(","));
    logFile.print(bms_status.temperature);
    logFile.print(F(","));
    logFile.print(F("PUT MOTOR TEMP HERE"));//TODO: PUT MOTOR TEMP HERE
    logFile.print(F(","));
    logFile.print(bms_status.voltage);
    logFile.println();

    logFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println(F("error logging to SD"));
  }
}


void msgHandler(uint32_t frame_id, byte *frame_data, byte length) {
  switch (frame_id) {
    case CAN_ID_BRAKE:
      msgHandleBrake(frame_id, frame_data, length);
      break;
    case CAN_ID_HAZARD:
      msgHandleHazard(frame_id, frame_data, length);
      break;
    case CAN_ID_MOTOR_CTRL:
      msgHandleMotor(frame_id, frame_data, length);
      break;
    case CAN_ID_SPEED_SENSOR:
      msgHandleSpeed(frame_id, frame_data, length);
      break;
    case CAN_ID_SIGNAL_CTRL:
      msgHandleSignal(frame_id, frame_data, length);
      break;
    case CAN_ID_ZEVA_BMS_CORE_STATUS:
      msgHandleCoreStatus(frame_id, frame_data, length);
      break;
    //case CAN_ID_ZEVA_BMS_RESET_SOC:
    //printLogHeader(1);
    //printHelper("State of Charge reseted", NEW_LINE);
    // break; //TODO: log
    default:
      if (frame_id >= CAN_ID_ZEVA_BMS_BASE && frame_id < 140) {
        msgHandleBms(frame_id, frame_data, length);
      }
      break;
  }
  logToSD();
}
