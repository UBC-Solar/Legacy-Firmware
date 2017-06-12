void printLogHeader(int frame_id) {
  myRTC.updateTime();
  printHelper(F("["));
  printHelper(String(myRTC.hours));
  printHelper(F(":"));
  printHelper(String(myRTC.minutes));
  printHelper(F(":"));
  printHelper(String(myRTC.seconds));
  printHelper(F("]{"));
  printHelper(String(frame_id));
  printHelper("} ");
}


void SD_init() {
  myRTC.updateTime();
  int month = myRTC.month;
  
  int day = myRTC.dayofmonth;
  int hour = myRTC.hours;
  int minute = myRTC.minutes;
  
  char logFilenameArr[13];
  sprintf(logFilenameArr, "%02u%02u%02u%02u.csv", month, day, hour, minute);
  logFilename = logFilenameArr;
  
  Serial.print(F("logFilename: "));
  Serial.println(logFilename);

  char serialFilenameArr[13];
  sprintf(serialFilenameArr, "%02u%02u%02u%02u.txt", month, day, hour, minute);
  serialFilename = serialFilenameArr;

  Serial.print(F("serialFilename: "));
  Serial.println(serialFilename);
  
  Serial.print(F("Initializing SD card..."));

  if (!SD.begin(4)) {
    Serial.println(F("failed!"));
    return;
  }
  Serial.println(F("done."));
}

void printHelper(String message, boolean newLine){
  if(newLine) {
    
    serialFile = SD.open(serialFilename, FILE_WRITE);
    if(serialFile){
      serialFile.println(message);
      serialFile.close();
    } else {
    //  Serial.println(F("unable to open serial file"));
    }
    Serial.println(message);
  } else {
    
    serialFile = SD.open(serialFilename, FILE_WRITE);
    if(serialFile){
      serialFile.print(message);
      serialFile.close();
    } else {
     // Serial.println(F("unable to open serial file"));
    }
    Serial.print(message);
  }
}
