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
  
  if(messageID != 3 && messageID != 5) {
    Serial.print(F("BMS #"));
    Serial.print((frame_id-100)/10);
    Serial.print(F(" packet "));
    Serial.print(frame_id%10);
    Serial.println(F(" parsing not implemented"));
    return;
  }
  
  byte bmsId=(frame_id-100)/10;
  byte voltGrp=(frame_id%10)/2-1;
  
  Serial.print(F("BMS #"));
  Serial.print(bmsId);
  for(int i=0; i<6; i++) {
    Serial.print(F(" c"));
    Serial.print(i+6*voltGrp);
    Serial.print(F("="));
    Serial.print(frame_data[i]+((frame_data[6]>>i)&1 ? 256 : 0));
  }
  Serial.print(F(" t"));
  Serial.print(voltGrp);
  Serial.print(F("="));
  Serial.print(frame_data[7] - 128);
  Serial.println();
}

void msgHandleZevaCoreStatus(uint32_t frame_id, byte *frame_data, byte length) {
  Serial.println(F("BMS Core status packet"));
  Serial.print(F("Status: "));
  Serial.println(frame_data[0]&15);
  Serial.print(F("Error: "));
  Serial.println(frame_data[0]>>4);
  Serial.print(F("State of Charge: "));
  Serial.println(frame_data[1]);
  Serial.print(F("Voltage: "));
  Serial.println(frame_data[2] + ((frame_data[3] & 0x0F) << 8));
  Serial.print(F("Current: "));
  Serial.println(((frame_data[4] << 4) + ((frame_data[3] & 0xF0) >> 4)) - 2048);
  Serial.print(F("Auxiliary Voltage: "));
  Serial.println(frame_data[5]/10.0);
  Serial.print(F("Temperature: "));
  Serial.println(frame_data[7]);
}

void msgHandleZevaCoreConfigData1(uint32_t frame_id, byte *frame_data, byte length) {  
  Serial.print(F("Pack capacity [Ah]: "));
  Serial.println(frame_data[0]);
  Serial.print(F("SOC warning threshold [%]: "));
  Serial.println(frame_data[1]);
  Serial.print(F("Full voltage [V]: "));
  Serial.println(2 * frame_data[2]);
  Serial.print(F("Current warning threshold [A]: "));
  Serial.println(10 * frame_data[3]);
  Serial.print(F("Overcurrent threshold [A]: "));
  Serial.println(10 * frame_data[4]);
  Serial.print(F("Over-temperature theshold [C]: "));
  Serial.println(frame_data[5]);
  Serial.print(F("Minimum auxiliary voltage [V]: "));
  Serial.println(frame_data[6]);
  Serial.print(F("Maximum leakage [%]: "));
  Serial.println(frame_data[7]);
}

void msgHandleZevaCoreConfigData2(uint32_t frame_id, byte *frame_data, byte length) {
  Serial.print(F("Tacho pulses-per-rev (for gauge scaling): "));
  Serial.println(frame_data[0]);
  Serial.print(F("Fuel gauge full (for gauge scaling): "));
  Serial.println(frame_data[1]);
  Serial.print(F("Fuel gauge empty: "));
  Serial.println(frame_data[2]);
  Serial.print(F("Temp gauge hot: "));
  Serial.println(frame_data[3]);
  Serial.print(F("Temp gauge cold: "));
  Serial.println(frame_data[4]);
  Serial.print(F("Peukerts exponent: "));
  Serial.println(frame_data[5]);
  Serial.print(F("Enable precharge: "));
  Serial.println(frame_data[6]);
  Serial.print(F("Enable contactor auxiliary switches: "));
  Serial.println(frame_data[7]);
}
  
void msgHandleZevaCoreConfigData3(uint32_t frame_id, byte *frame_data, byte length) {
  Serial.print(F("BMS minimum cell voltage: "));
  Serial.println(frame_data[0]);
  Serial.print(F("BMS maximum cell voltage: "));
  Serial.println(frame_data[1]);
  Serial.print(F("BMS shunt voltage: "));
  Serial.println(frame_data[2]);
  Serial.print(F("Low temperature warning: "));
  Serial.println(frame_data[3]);
  Serial.print(F("High temperature warning: "));
  Serial.println(frame_data[4]);
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

