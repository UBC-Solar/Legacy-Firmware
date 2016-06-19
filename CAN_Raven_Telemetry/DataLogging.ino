#include <SdFat.h>

// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
#define SD_SS 4

SdFat sd;
SdFile logfile;

void log_onfileerror(void){
  Serial.println(F("logfile IO failed"));
  logfile.close();
}

int log_openlogfile(void){
  struct datetime dt;
  ds1302_readtime(&dt);
  char filename[20];
  snprintf(filename, 20, "%02d%02d%02d.can", dt.year, dt.month, dt.day);
  
  if (!logfile.open(filename, O_CREAT | O_APPEND | O_WRITE)) {
    Serial.print(F("cannot open logfile "));
    Serial.println(filename);
    return 0;
  }
  Serial.println(F("logfile successfully opened"));
  return 1;
}

void log_init(void){
  Serial.print(F("\nInitializing SD card..."));

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!sd.begin(SD_SS, SPI_HALF_SPEED)) {
    Serial.println(F("initialization failed. Things to check:"));
    Serial.println(F("* is a card inserted?"));
    Serial.println(F("* is your wiring correct?"));
    Serial.println(F("* did you change the chipSelect pin to match your shield or module?"));
    return;
  }
  
  Serial.println(F("SD card wiring is correct and a card is present."));

  log_openlogfile();
}

void log_can(uint32_t id, byte length, const byte *data){
  if(!logfile.isOpen()){
    return;
  }

  struct datetime dt;
  ds1302_readtime(&dt);

  if(logfile.write(&dt, sizeof(dt)) < 0){
    log_onfileerror();
  }
  if(logfile.write(&id, sizeof(id)) < 0){
    log_onfileerror();
  }
  if(logfile.write(length) < 0){
    log_onfileerror();
  }
  if(logfile.write(data, length) < 0){
    log_onfileerror();
  }

  logfile.sync();
}

