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

void zevaCoreSendConfigData1() {
  byte length = 8;
  uint32_t frame_id = CAN_ID_ZEVA_BMS_CORE_CONFIG_WR1;
  byte frame_data[8];

  frame_data[0] = 80;     // pack capacity is 20 Ah
  frame_data[1] = 20;     // low state of charge warning is 20%
  frame_data[2] = int(12.54/2);   // max pack voltage is 46V
  frame_data[3] = 50/10; // overcurrent warning threshold is 50A (max. continuous discharge)
  frame_data[4] = 100/10; // overcurrent shutdown threshold is 100A (max. peak discharge)
  frame_data[5] = 40;     // overtemperature threshold is 40C
  frame_data[6] = 11;     // minimum auxiliary voltage (from DC/DC or supplemental battery) is 11V
  frame_data[7] = 50;     // maximum leakage percentage is 50%

  CAN.sendMsgBuf(frame_id, 0, length, frame_data);
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
  Serial.println(frame_data[5]/10.0);
  Serial.print(F("Enable precharge: "));
  Serial.println(frame_data[6]);
  Serial.print(F("Enable contactor auxiliary switches: "));
  Serial.println(frame_data[7]);
}

void zevaCoreSendConfigData2() {
  byte length = 8;
  uint32_t frame_id = CAN_ID_ZEVA_BMS_CORE_CONFIG_WR2;
  byte frame_data[8];

  frame_data[0] = 1;            // tachometer pulses per revolution (we aren't using this feature)
  frame_data[1] = 100;          // fuel gauge full percentage (we aren't using this feature)
  frame_data[2] = 0;            // fuel gauge empty percentage (we aren't using this feature)
  frame_data[3] = 100;          // temp gauge hot percentage (we aren't using this feature)
  frame_data[4] = 0;            // temp gauge cold percentage (we aren't using this feature)
  frame_data[5] = int(1.2*10);  // peukeurt's exponent
  frame_data[6] = 0;            // enable precharge (0 = disabled, 1 = enabled)
  frame_data[7] = 1;            // enable contactor auxiliary switches (0 = disabled, 1 = enabled)

  CAN.sendMsgBuf(frame_id, 0, length, frame_data);
}
  
void msgHandleZevaCoreConfigData3(uint32_t frame_id, byte *frame_data, byte length) {
  Serial.print(F("BMS minimum cell voltage: "));
  Serial.println(frame_data[0]*0.05);
  Serial.print(F("BMS maximum cell voltage: "));
  Serial.println(frame_data[1]*0.05);
  Serial.print(F("BMS shunt voltage: "));
  Serial.println(frame_data[2]*0.05);
  Serial.print(F("Low temperature warning: "));
  Serial.println(frame_data[3]);
  Serial.print(F("High temperature warning: "));
  Serial.println(frame_data[4]);
}

void zevaCoreSendConfigData3() {
  byte length = 8;
  uint32_t frame_id = CAN_ID_ZEVA_BMS_CORE_CONFIG_WR3;
  byte frame_data[8];

  frame_data[0] = int(3.0/0.05);  // bms minimum cell voltage is 3.0V (50 mV increments)
  frame_data[1] = int(4.15/0.05); // bms maximum cell voltage is 4.15V (50 mV increments)
  frame_data[2] = int(3.8/0.05);  // bms shunt balancers turn on at 3.8V (50 mV increments)
  frame_data[3] = 0;              // low temperature warning threshold is 0C
  frame_data[4] = 40;             // high temperature warning threshold is 40C

  CAN.sendMsgBuf(frame_id, 0, length, frame_data);
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

void zevaCoreSetCellNum(void) {
  byte length = 8;
  uint32_t frame_id = CAN_ID_ZEVA_BMS_CORE_SET_CELL_NUM;
  byte frame_data[8];

  // format is 16 4-bit values
  // ie. 0x0B means module ID 0 has 11 cells
  //     0xBB means module IDs 0 and 1 have 11 cells each
  frame_data[0] = 0x33;
  frame_data[1] = 0x23;
  frame_data[2] = 0;
  frame_data[3] = 0;
  frame_data[4] = 0;
  frame_data[5] = 0;
  frame_data[6] = 0;
  frame_data[7] = 0;
  
  CAN.sendMsgBuf(frame_id, 0, length, frame_data);
}

void zevaCoreStartSetupMode(void) {
  byte length = 1;
  uint32_t frame_id = CAN_ID_ZEVA_BMS_CORE_SET_STATE;
  byte frame_data[1];
  frame_data[0] = 1;
  CAN.sendMsgBuf(frame_id, 0, length, frame_data);
}

void zevaCoreEndSetupMode(void) {
  byte length = 1;
  uint32_t frame_id = CAN_ID_ZEVA_BMS_CORE_SET_STATE;
  byte frame_data[1];
  frame_data[0] = 0;
  CAN.sendMsgBuf(frame_id, 0, length, frame_data);
}

