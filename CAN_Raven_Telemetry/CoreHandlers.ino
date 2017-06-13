void msgHandleHeartbeat(uint32_t frame_id, byte* frame_data, byte length) {
  printLogHeader(frame_id);
  printHelper(F("A"));
  printHelper(String(frame_data[0]));
  printHelper(F("R"));
  printHelper(String(frame_data[1]));
  printHelper(F("D"));
  printHelper(String(frame_data[2]));
  printHelper(F("S"));
  printHelper(String(frame_data[3]));
  printHelper(F("E"),1);
}

void msgHandleBrake(uint32_t frame_id, byte *frame_data, byte length) {
  printLogHeader(frame_id);
  printHelper(String(frame_data[0]), 1);
}

void msgHandleCoreStatus(uint32_t frame_id, byte* frame_data, byte length) {
  byte status = frame_data[0] >> 4;
  byte error = frame_data[0] & 0xF;
  packet.soc = frame_data[1];
  packet.voltage  = ((unsigned long) frame_data[2]) | (((unsigned long) frame_data[3] & 0xF0) << 4);
  int current = (((unsigned long) frame_data[3] & 0xF) | ((unsigned long) frame_data[4] << 4)) - 2048;
  packet.temperature = frame_data[7];
  printLogHeader(frame_id);
  printHelper(String(status) + " ");
  printHelper(String(error) + " ");
  printHelper(String(packet.soc) + " ");
  printHelper(String(packet.voltage) + " ");
  printHelper(String(current) + " ");
  printHelper(String(frame_data[5]/10.0) + " ");
  printHelper(String(packet.temperature), NEW_LINE);
}

void msgHandleBmsStatus(uint32_t frame_id, byte* frame_data, byte length) {
  int pack_num = (frame_id % 100) / 10;

  uint32_t volt_warn = ((unsigned long) frame_data[2]) << 16 | ((unsigned long) frame_data[1]) << 8 | ((unsigned long) frame_data[0]);
  uint16_t volt_shun_warn = (frame_data[4] & 0x0F) << 8 | (unsigned char) frame_data[3];
  byte temp_warn = frame_data[4] >> 4;

  for (int i = 0; i < 12; i++) {
    printHelper(String((volt_warn >> i) & 0x1));
    printHelper(String((volt_warn >> i + 12) & 0x1));
    printHelper(String((volt_shun_warn >> i) & 0x1));
    printHelper(F(" "));
  }
  printHelper(String(temp_warn & 0x3));
  printHelper(" ");
  printHelper(String(temp_warn & 0xC), 1);
}

void msgHandleBmsReply(uint32_t frame_id, byte* frame_data, byte length) {
  for (int i = 0 ; i < 6 ; i++) {
    unsigned int cell_volt = ((unsigned int) frame_data[i]) | (frame_data[6] & ((unsigned int) 1 << i)) << (8 - i);
    printHelper(String(cell_volt/100.0));
    printHelper(F(" "));
  }
  byte temp = frame_data[7] - 128;
  printHelper(String(temp), NEW_LINE);
}

void msgHandleBms(uint32_t frame_id, byte* frame_data, byte length) {
  printLogHeader(frame_id);
  if ((frame_id % 10) % 2 == 0) {
    printHelper("", 1);
  }

  else if (frame_id % 10 == 1) {
    msgHandleBmsStatus(frame_id, frame_data, length);
  }

  else if ((frame_id % 10) == 3 || (frame_id % 10) == 5) {
    msgHandleBmsReply(frame_id, frame_data, length);
  }
}

void msgHandleMPPTData(uint32_t frame_id, byte* frame_data, byte length) {
  printLogHeader(frame_id);
  int numValues;
  if(frame_id == CAN_ID_CURRENT_SENSOR_2 || frame_id == CAN_ID_TEMP_SENSOR_3) {
    numValues = 2;
  } else {
    numValues = 4;
  }

  for(int i = 0; i < numValues * 2; i += 2) {
    double sensorValue = word(frame_data[i], frame_data[i+1]) / 100.0;
    printHelper(String(sensorValue));
    printHelper(F(" "));
  }
  printHelper("", 1);
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
    case CAN_ID_HEARTBEAT:
      msgHandleHeartbeat(frame_id, frame_data, length);
      break;
    case CAN_ID_BRAKE:
      msgHandleBrake(frame_id, frame_data, length);
      break;
    case CAN_ID_ZEVA_BMS_CORE_STATUS:
      msgHandleCoreStatus(frame_id, frame_data, length);
      break;
    case CAN_ID_CURRENT_SENSOR_1:
      msgHandleMPPTData(frame_id, frame_data, length);
      break;
    case CAN_ID_CURRENT_SENSOR_2:
      msgHandleMPPTData(frame_id, frame_data, length);
      break;
    case CAN_ID_TEMP_SENSOR_1:
      msgHandleMPPTData(frame_id, frame_data, length);
      break;
    case CAN_ID_TEMP_SENSOR_2:
      msgHandleMPPTData(frame_id, frame_data, length);
      break;
    case CAN_ID_TEMP_SENSOR_3:
      msgHandleMPPTData(frame_id, frame_data, length);
      break;
    default:
      if (frame_id >= CAN_ID_ZEVA_BMS_BASE && frame_id < 140) {
        msgHandleBms(frame_id, frame_data, length);
      }
      break;
  }
  //logToSD();
}
