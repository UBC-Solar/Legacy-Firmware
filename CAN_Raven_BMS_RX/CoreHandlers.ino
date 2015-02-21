struct BMSConfig {
  byte pack_capacity;
  byte soc_warn_thresh;
  byte full_voltage; // x2 to get real value
  byte current_warn_thresh; // x10 to get real value
  byte overcurrent_thresh; // x10 to get real value
  byte overtemperature_thresh;
  byte min_aux_voltage;
  byte max_leakage;
};

BMSConfig bmsConfig;

void msgHandleZevaCoreStatus(byte rx_status, byte length, uint32_t frame_id, byte filter, byte buffer, byte *frame_data, byte ext) {
  
  int soc = frame_data[1];
  int voltage = (frame_data[2] + ((frame_data[3] & 0xF0) << 4));
  int current = ((frame_data[4] << 4) + (frame_data[3] & 0x0F)) - 2048;
  float aux_voltage = frame_data[5]/10;
  int temperature = frame_data[7];
  
  Serial.print("Error: ");
  Serial.println(frame_data[0]>>4);
  Serial.print("Status: ");
  Serial.println(frame_data[0]&15);
  Serial.print("State of Charge: ");
  Serial.println(soc);
  Serial.print("Voltage: ");
  Serial.println(voltage);
  Serial.print("Current: ");
  Serial.println(current);
  Serial.print("Auxiliary Voltage: ");
  Serial.println(aux_voltage);
  Serial.print("Temperature: ");
  Serial.println(temperature);
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

void msgHandleZevaCoreConfig(byte rx_status, byte length, uint32_t frame_id, byte filter, byte buffer, byte *frame_data, byte ext) {
  switch(frame_id){
    case 13:
      msgHandleZevaCoreConfigData1(rx_status, length, frame_id, filter, buffer, frame_data, ext);
      break;
    default:
      break;
  }
}
      
