void msgHandleBrake(uint32_t frame_id, byte *frame_data, byte length) {
  printLogHeader(0);
  printHelper(F("Brake: "));
  printONOFF(frame_data[0], true);
}

void msgHandleHazard(uint32_t frame_id, byte* frame_data, byte length) {
  printLogHeader(0);
  printHelper(F("Hazard: "));

  printONOFF(frame_data[0], true);
}

void msgHandleMotor(uint32_t frame_id, byte* frame_data, byte length) {
  byte throttle = frame_data[0];
  byte regen = frame_data[1];
  byte dir = frame_data[2];

  printLogHeader(0);
  printHelper(F("Throttle: "));
  printHelper(String(throttle));
  printHelper(F("  Regen: "));
  printHelper(String(regen));
  printHelper(F("  Dir: "));
  printHelper(String(dir), NEW_LINE);
}

void msgHandleSpeed(uint32_t frame_id, byte* frame_data, byte length) {
  unsigned long temp = ((unsigned long) frame_data[0] << 24) | ((unsigned long) frame_data[1] << 16) | ((unsigned long) frame_data[2] << 8) | ((unsigned long) frame_data[3]);
 // float freq = *((float*) &temp);
  printLogHeader(0);
  printHelper(F("Freq: "));

 // printHelper(String(freq));
  printHelper(F(" rps"), NEW_LINE);
}

void msgHandleSignal(uint32_t frame_id, byte* frame_data, byte length) {
  byte left_signal = frame_data[0] & 0x1;
  byte right_signal = frame_data[0] & 0x2;
  
  printLogHeader(0);
  printHelper(F("Left signal: "));
  printONOFF(left_signal, false);
  printHelper(F("  Right signal: "));
  printONOFF(right_signal, true);
}

void msgHandleCoreStatus(uint32_t frame_id, byte* frame_data, byte length) {
  byte status = frame_data[0] >> 4;
  byte error = frame_data[0] & 0xF;
  packet.soc = frame_data[1];
  packet.voltage  = ((unsigned long) frame_data[2]) | (((unsigned long) frame_data[3] & 0xF0) << 4);
  int current = (((unsigned long) frame_data[3] & 0xF) | ((unsigned long) frame_data[4] << 4)) - 2048;
  float aux_voltage = frame_data[5];
  packet.temperature = frame_data[7];

  if (!error) {
    printLogHeader(0);
  }
  else if (error == 2 || error == 4 || error == 6 || error == 9) {
    printLogHeader(1);
  }
  else {
    printLogHeader(2);
  }
  printHelper(F("Status: "));
  printHelper(String(status));
  printHelper(F("   Err: "));
  printHelper(String(error));
  printHelper(F("  Volt: "));
  printHelper(String(packet.voltage));
  printHelper(F("  Curr: "));
  printHelper(String(current));
  printHelper(F("  Aux Volt: "));
  printHelper(String(aux_voltage));
  printHelper(F("  Temp: "));
  printHelper(String(packet.temperature), NEW_LINE);
}

void msgHandleBmsStatus(uint32_t frame_id, byte* frame_data, byte length) {
  int pack_num = (frame_id % 100) / 10;

  uint32_t volt_warn = ((unsigned long) frame_data[2]) << 16 | ((unsigned long) frame_data[1]) << 8 | ((unsigned long) frame_data[0]);
  uint16_t volt_shun_warn = (frame_data[4] & 0x0F) << 8 | (unsigned char) frame_data[3];
  byte temp_warn = frame_data[4] >> 4;
  printLogHeader((volt_warn |volt_shun_warn | temp_warn) == 0 ? 0 : 2);
  printHelper(F("Status of battery pack "));
  printHelper(String(pack_num) + ": ", NEW_LINE);


  for (int i = 0; i < 12; i++) {
    printHelper("Cell " + String(i) + ": ");
    printHelper("Voltage: ");
    printHelper(String(volt_warn & ((unsigned long) 0x1 << i))/* == 0 ? "OK" : (volt_warn & ((unsigned long) 0x1 << i)) == 0 ? "HIGH" : "LOW"*/);
    printHelper("/");
    printHelper(String(volt_warn & ((unsigned long) 0x1000 << i))/* == 0 ? "OK" : (volt_warn & ((unsigned long) 0x1 << i)) == 0 ? "HIGH" : "LOW"*/);
    printHelper("/");
    printHelper(String(volt_shun_warn & (unsigned int) 0x1 << i)/* == 0 ? "OK  " : "SHUN  ", (i%4 == 3) ? NEW_LINE : 0*/);
  }

  for (int i = 0; i < 2; i++) {
    printHelper("Temperature " + String(i) + ": ");
    printHelper(String(temp_warn & (0x5 << i))/* == 0 ? "OK  " : ((temp_warn & ( 0x1 << i*2)) == 0 ? "HIGH  " : "LOW  ")*/, i);
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
    unsigned int cell_volt = ((unsigned int) frame_data[i % 6]) | (frame_data[6] & ((unsigned int) 1 << i % 6)) << 8 - i % 6;

    printHelper(F(" "));
    printHelper(String(i));
    printHelper(F(": "));
    printHelper(String(cell_volt));
  }
  byte temp = frame_data[7] - 128;

  printHelper(F(" Temp: "));
  printHelper(String(temp), NEW_LINE);
}

void msgHandleBms(uint32_t frame_id, byte* frame_data, byte length) {
  if ((frame_id % 10) % 2 == 0) {
    printLogHeader(3);
    printHelper(" Requesting ");
    printHelper((frame_id % 10 == 0) ? "Status" : "voltage " /*+ String(frame_id % 10 / 2)*/, NEW_LINE);
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
    logFile.print(String(packet.speed));//TODO: PUT SPEED HERE
    logFile.print(F(","));
    logFile.print(packet.soc);
    logFile.print(F(","));
    logFile.print(packet.temperature);
    logFile.print(F(","));
    logFile.print(packet.motor_temp);//TODO: PUT MOTOR TEMP HERE
    logFile.print(F(","));
    logFile.print(packet.voltage);
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
  //logToSD();
}
