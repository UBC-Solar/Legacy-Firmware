#include <ubcsolar_can_ids.h>

void msgHandleZevaBms(uint32_t frame_id, byte *frame_data, byte length) {
  
  uint32_t messageID = frame_id%10;
  
  // even IDs are requests from Core to BMS12
  if(messageID%2 == 0) {
    
    switch (messageID) {
      case 0: Serial.print(F("Request for status from Core to Module ")); break;
      case 2: Serial.print(F("Request for voltages #1 from Core to Module ")); break;
      case 4: Serial.print(F("Request for voltages #2 from Core to Module ")); break;
      case 6: Serial.print(F("Request for config info from Core to Module ")); break;
      case 8: Serial.print(F("Command to set config info from Core to Module ")); break;
      default: Serial.print(F("Unknown message from Core to Module ")); break;
    }
    Serial.print((frame_id - 100 - messageID)/10);  // frameID = 100 + 10 * moduleID + messageID, 0 <= messageID <=8
    Serial.println();

    return;
  }
  
  if(messageID != 3 && messageID != 5){
    Serial.print(F("BMS #"));
    Serial.print((frame_id-100)/10);
    Serial.print(F(" packet "));
    Serial.print(frame_id%10);
    Serial.println(F(" parsing not implemented"));
    return;
  }
  
  byte bmsId=(frame_id-100)/10;
  byte voltGrp=(frame_id%10)/2-1;
  
  for(int i=0; i<6; i++) {
    cellVoltagesX100[bmsId][voltGrp*6+i] = frame_data[i]+((frame_data[6]>>i)&1 ? 256 : 0);
  }
  bmsTemperatures[bmsId][voltGrp] = frame_data[7] - 128;
  
  Serial.print(F("BMS #"));
  Serial.print(bmsId);
  for(int i=0; i<6; i++) {
    Serial.print(F(" c"));
    Serial.print(i+6*voltGrp);
    Serial.print(F("="));
    Serial.print(cellVoltagesX100[bmsId][voltGrp*6+i]);
  }
  Serial.print(F(" t"));
  Serial.print(voltGrp);
  Serial.print(F("="));
  Serial.print(bmsTemperatures[bmsId][voltGrp]);
  Serial.println();
}

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
  
  Serial.println(F("BMS Core status packet"));
  zevaCoreStatusPrint();
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

  // format is 16 4-bit values
  // ie. 0x0B means module ID 0 has 11 cells
  //     0xBB means module IDs 0 and 1 have 11 cells each
  
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
}

void zevaCoreStartSetupMode(void){
  byte length = 1;
  uint32_t frame_id = CAN_ID_ZEVA_BMS_CORE_SET_STATE;
  byte frame_data[1];
  frame_data[0] = 1;
  CAN.sendMsgBuf(frame_id, 0, length, frame_data);
}

void zevaCoreEndSetupMode(void){
  byte length = 1;
  uint32_t frame_id = CAN_ID_ZEVA_BMS_CORE_SET_STATE;
  byte frame_data[1];
  frame_data[0] = 0;
  CAN.sendMsgBuf(frame_id, 0, length, frame_data);
}

