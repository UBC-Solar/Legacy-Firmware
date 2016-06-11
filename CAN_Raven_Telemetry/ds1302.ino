// Code to interact with the DS1302 RTC

#define DS1302PIN 6
#define DS1302_MAXCLK 1000000 // spec sheet says up to 2.0MHz, but that doesn't work.

#define DS1302_WR 0
#define DS1302_RD 1

#define DS1302_CHARGECFG 0x90
#define DS1302_WRITEPROT 0x8e
#define DS1302_YEAR 0x8c
#define DS1302_WEEKDAY 0x8a
#define DS1302_MONTH 0x88
#define DS1302_DAY 0x86
#define DS1302_HOUR 0x84
#define DS1302_MINUTE 0x82
#define DS1302_SECOND 0x80

#define DS1302_CLOCKBURST 0xbe

byte ds1302_io(byte addr, byte data){
  byte retval;
  SPI.beginTransaction(SPISettings(DS1302_MAXCLK, LSBFIRST, SPI_MODE0));
  digitalWrite(DS1302PIN, HIGH);
  SPI.transfer(addr);
  retval = SPI.transfer(data);
  digitalWrite(DS1302PIN, LOW);
  SPI.endTransaction();
  return retval;
}

void ds1302_init(void){
  pinMode(DS1302PIN, OUTPUT);
  digitalWrite(DS1302PIN, LOW);
  delay(1);
  ds1302_io(DS1302_CHARGECFG | DS1302_WR, 0xA9); //enable trickle charger, 2 diodes, 2k ohm resistor
}

void ds1302_readtime(struct datetime *dt){
  byte temp;
  SPI.beginTransaction(SPISettings(DS1302_MAXCLK, LSBFIRST, SPI_MODE0));
  digitalWrite(DS1302PIN, HIGH);
  SPI.transfer(DS1302_CLOCKBURST | DS1302_RD); //start burst
  temp = SPI.transfer(0x00);
  dt->second = (temp & 0x0f) + ((temp & 0x70) >> 4) * 10;
  temp = SPI.transfer(0x00);
  dt->minute = (temp & 0x0f) + ((temp & 0x70) >> 4) * 10;
  temp = SPI.transfer(0x00);
  dt->hour = (temp & 0x0f) + ((temp & 0x30) >> 4) * 10;
  temp = SPI.transfer(0x00);
  dt->day = (temp & 0x0f) + ((temp & 0x30) >> 4) * 10;
  temp = SPI.transfer(0x00);
  dt->month = (temp & 0x0f) + ((temp & 0x10) >> 4) * 10;
  temp = SPI.transfer(0x00); // ignore weekday
  temp = SPI.transfer(0x00);
  dt->year = (temp & 0x0f) + ((temp & 0xf0) >> 4) * 10;
  temp = SPI.transfer(0x00); // ignore write protect
  digitalWrite(DS1302PIN, LOW);
  SPI.endTransaction();
}

void ds1302_writetime(struct datetime *dt){
  byte temp;
  SPI.beginTransaction(SPISettings(DS1302_MAXCLK, LSBFIRST, SPI_MODE0));
  digitalWrite(DS1302PIN, HIGH);
  SPI.transfer(DS1302_CLOCKBURST | DS1302_WR); //start burst
  temp = (dt->second % 10) | ((dt->second / 10) << 4);
  SPI.transfer(temp);
  temp = (dt->minute % 10) | ((dt->minute / 10) << 4);
  SPI.transfer(temp);
  temp = (dt->hour % 10) | ((dt->hour / 10) << 4);
  SPI.transfer(temp);
  temp = (dt->day % 10) | ((dt->day / 10) << 4);
  SPI.transfer(temp);
  temp = (dt->month % 10) | ((dt->month / 10) << 4);
  SPI.transfer(temp);
  SPI.transfer(1); // dummy weekday
  temp = (dt->year % 10) | ((dt->year / 10) << 4);
  SPI.transfer(temp);
  SPI.transfer(0x00); // write protect off
  digitalWrite(DS1302PIN, LOW);
  SPI.endTransaction();
}

void print_time(struct datetime *dt){
  char tmp[20];
  snprintf(tmp, 20, "%02d/%02d/%02d %02d:%02d:%02d", dt->year, dt->month, dt->day, dt->hour, dt->minute, dt->second);
  Serial.println(tmp);
}
