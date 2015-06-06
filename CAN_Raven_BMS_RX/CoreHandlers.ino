void zevaCoreStatusPrint(){
  Serial.print("Error: ");
  Serial.println(bmsStatus.error);
  Serial.print("Status: ");
  Serial.println(bmsStatus.status);
  Serial.print("State of Charge: ");
  Serial.println(bmsStatus.soc);
  Serial.print("Voltage: ");
  Serial.println(bmsStatus.voltage);
  Serial.print("Current: ");
  Serial.println(bmsStatus.current);
  Serial.print("Auxiliary Voltage: ");
  Serial.println(bmsStatus.aux_voltage);
  Serial.print("Temperature: ");
  Serial.println(bmsStatus.temperature);
}

void msgHandleZevaCoreStatus(byte rx_status, byte length, uint32_t frame_id, byte filter, byte buffer, byte *frame_data, byte ext) {
  
  bmsStatus.status = frame_data[0]&15;
  bmsStatus.error = frame_data[0]>>4;
  bmsStatus.soc = frame_data[1];
  bmsStatus.voltage = (frame_data[2] + ((frame_data[3] & 0x0F) << 8));
  bmsStatus.current = ((frame_data[4] << 4) + ((frame_data[3] & 0xF0) >> 4)) - 2048;
  bmsStatus.aux_voltage = frame_data[5]/10.0;
  bmsStatus.temperature = frame_data[7];
  
  #if DEBUG
  Serial.println("BMS Core status packet");
  zevaCoreStatusPrint();
  #endif
  
  bmsAlive |= 1;
}

void msgHandleZevaCoreConfigData1(byte rx_status, byte length, uint32_t frame_id, byte filter, byte buffer, byte *frame_data, byte ext) {
  bmsConfig.pack_capacity = frame_data[0];
  bmsConfig.soc_warn_thresh = frame_data[1];
  bmsConfig.full_voltage = frame_data[2];
  bmsConfig.current_warn_thresh = frame_data[3];
  bmsConfig.overcurrent_thresh = frame_data[4];
  bmsConfig.overtemperature_thresh = frame_data[5];
  bmsConfig.min_aux_voltage = frame_data[6];
  bmsConfig.max_leakage = frame_data[7];
  bmsConfig.valid |= 1;
  
  Serial.print("Pack capacity [Ah]: ");
  Serial.println(bmsConfig.pack_capacity);
  
  Serial.print("SOC warning threshold [%]: ");
  Serial.println(bmsConfig.soc_warn_thresh);
  
  Serial.print("Full voltage [V]: ");
  Serial.println(2 * bmsConfig.full_voltage);
  
  Serial.print("Current warning threshold [A]: ");
  Serial.println(10 * bmsConfig.current_warn_thresh);
  
  Serial.print("Overcurrent threshold [A]: ");
  Serial.println(10 * bmsConfig.overcurrent_thresh);
  
  Serial.print("Over-temperature theshold [C]: ");
  Serial.println(bmsConfig.overtemperature_thresh);
  
  Serial.print("Minimum auxiliary voltage [V]: ");
  Serial.println(bmsConfig.min_aux_voltage);
  
  Serial.print("Maximum leakage [%]: ");
  Serial.println(bmsConfig.max_leakage);
}

void msgHandleZevaCoreConfigData2(byte rx_status, byte length, uint32_t frame_id, byte filter, byte buffer, byte *frame_data, byte ext) {
  bmsConfig.tacho_pulses_per_rev = frame_data[0];
  bmsConfig.fuel_gauge_full = frame_data[1];
  bmsConfig.fuel_gauge_empty = frame_data[2];
  bmsConfig.temp_gauge_hot = frame_data[3];
  bmsConfig.temp_gauge_cold = frame_data[4];
  bmsConfig.peukerts_exponent = frame_data[5];
  bmsConfig.enable_precharge = frame_data[6];
  bmsConfig.enable_contactor_aux_sw = frame_data[7];
  bmsConfig.valid |= 2;
  
  Serial.print("Tacho pulses-per-rev (for gauge scaling): ");
  Serial.println(bmsConfig.tacho_pulses_per_rev);
  Serial.print("Fuel gauge full (for gauge scaling): ");
  Serial.println(bmsConfig.fuel_gauge_full);
  Serial.print("Fuel gauge empty: ");
  Serial.println(bmsConfig.fuel_gauge_empty);
  Serial.print("Temp gauge hot: ");
  Serial.println(bmsConfig.temp_gauge_hot);
  Serial.print("Temp gauge cold: ");
  Serial.println(bmsConfig.temp_gauge_cold);
  Serial.print("Peukerts exponent: ");
  Serial.println(bmsConfig.peukerts_exponent);
  Serial.print("Enable precharge: ");
  Serial.println(bmsConfig.enable_precharge);
  Serial.print("Enable contactor auxiliary switches: ");
  Serial.println(bmsConfig.enable_contactor_aux_sw);
}
  
void msgHandleZevaCoreConfigData3(byte rx_status, byte length, uint32_t frame_id, byte filter, byte buffer, byte *frame_data, byte ext) {
  bmsConfig.bms_min_cell_voltage = frame_data[0];
  bmsConfig.bms_max_cell_voltage = frame_data[1];
  bmsConfig.bms_shunt_voltage = frame_data[2];
  bmsConfig.low_temperature_warn = frame_data[3];
  bmsConfig.high_temperature_warn = frame_data[4];
  bmsConfig.valid |= 4;
  
  Serial.print("BMS minimum cell voltage: ");
  Serial.println(bmsConfig.bms_min_cell_voltage);
  Serial.print("BMS maximum cell voltage: ");
  Serial.println(bmsConfig.bms_max_cell_voltage);
  Serial.print("BMS shunt voltage: ");
  Serial.println(bmsConfig.bms_shunt_voltage);
  Serial.print("Low temperature warning: ");
  Serial.println(bmsConfig.low_temperature_warn);
  Serial.print("High temperature warning: ");
  Serial.println(bmsConfig.high_temperature_warn);
}
  

void msgHandleZevaCoreConfig(byte rx_status, byte length, uint32_t frame_id, byte filter, byte buffer, byte *frame_data, byte ext) {
  switch(frame_id){
    case ZEVA_BMS_CORE_CONFIG_RD1:
      msgHandleZevaCoreConfigData1(rx_status, length, frame_id, filter, buffer, frame_data, ext);
      break;
    case ZEVA_BMS_CORE_CONFIG_RD2:
      msgHandleZevaCoreConfigData2(rx_status, length, frame_id, filter, buffer, frame_data, ext);
      break;
    case ZEVA_BMS_CORE_CONFIG_RD3:
      msgHandleZevaCoreConfigData3(rx_status, length, frame_id, filter, buffer, frame_data, ext);
      break;
    default:
      break;
  }
}

void zevaCoreSetCellNum(void){
  byte length,rx_status,filter,ext;
  uint32_t frame_id;
  byte frame_data[8];
  
  Serial.println("SEND CELL NUM");
  frame_id = ZEVA_BMS_CORE_SET_CELL_NUM;
  frame_data[0] = 0x0B;
  frame_data[1] = 0;
  frame_data[2] = 0;
  frame_data[3] = 0;
  frame_data[4] = 0;
  frame_data[5] = 0;
  frame_data[6] = 0;
  frame_data[7] = 0;
  length = 8;
  CAN.load_ff_0(length, &frame_id, frame_data, false);
}
