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

#include "RTCDriver.h"
#include <SPI.h>

RTCDriver::RTCDriver(uint8_t pin, uint32_t maxClock)
  : _pin(pin), _maxClock(maxClock) { }

void RTCDriver::begin() {
  SPI.begin();
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  delay(1);

  // enable trickle charger, diodes, 2kohm resistor
  uint8_t addr = DS1302_CHARGECFG | DS1302_WR;
  uint8_t data = 0xA9;
  SPI.beginTransaction(SPISettings(_maxClock, LSBFIRST, SPI_MODE0));
  digitalWrite(_pin, HIGH);
  SPI.transfer(addr);
  SPI.transfer(data);
  digitalWrite(_pin, LOW);
  SPI.endTransaction();
}

void RTCDriver::getTime(struct DateTime* dateTime) {
  uint8_t temp;
  SPI.beginTransaction(SPISettings(_maxClock, LSBFIRST, SPI_MODE0));
  digitalWrite(_pin, HIGH);
  SPI.transfer(DS1302_CLOCKBURST | DS1302_RD); //start burst
  temp = SPI.transfer(0x00);
  dateTime->second = (temp & 0x0f) + ((temp & 0x70) >> 4) * 10;
  temp = SPI.transfer(0x00);
  dateTime->minute = (temp & 0x0f) + ((temp & 0x70) >> 4) * 10;
  temp = SPI.transfer(0x00);
  dateTime->hour = (temp & 0x0f) + ((temp & 0x30) >> 4) * 10;
  temp = SPI.transfer(0x00);
  dateTime->day = (temp & 0x0f) + ((temp & 0x30) >> 4) * 10;
  temp = SPI.transfer(0x00);
  dateTime->month = (temp & 0x0f) + ((temp & 0x10) >> 4) * 10;
  temp = SPI.transfer(0x00); // ignore weekday
  temp = SPI.transfer(0x00);
  dateTime->year = (temp & 0x0f) + ((temp & 0xf0) >> 4) * 10;
  temp = SPI.transfer(0x00); // ignore write protect
  digitalWrite(_pin, LOW);
  SPI.endTransaction();
}

struct DateTime RTCDriver::getTime() {
  /*struct DateTime dateTime = {0, 0, 0, 0, 0, 0};
  RTCDriver::getTime(&dateTime);
  return dateTime;*/

  uint8_t temp;
  uint8_t ss, mm, hh, day, month, year;
  
  SPI.beginTransaction(SPISettings(_maxClock, LSBFIRST, SPI_MODE0));
  digitalWrite(_pin, HIGH);
  SPI.transfer(DS1302_CLOCKBURST | DS1302_RD); //start burst
  temp = SPI.transfer(0x00);
  ss = (temp & 0x0f) + ((temp & 0x70) >> 4) * 10;
  temp = SPI.transfer(0x00);
  mm = (temp & 0x0f) + ((temp & 0x70) >> 4) * 10;
  temp = SPI.transfer(0x00);
  hh = (temp & 0x0f) + ((temp & 0x30) >> 4) * 10;
  temp = SPI.transfer(0x00);
  day = (temp & 0x0f) + ((temp & 0x30) >> 4) * 10;
  temp = SPI.transfer(0x00);
  month = (temp & 0x0f) + ((temp & 0x10) >> 4) * 10;
  temp = SPI.transfer(0x00); // ignore weekday
  temp = SPI.transfer(0x00);
  year = (temp & 0x0f) + ((temp & 0xf0) >> 4) * 10;
  temp = SPI.transfer(0x00); // ignore write protect
  digitalWrite(_pin, LOW);
  SPI.endTransaction();

  struct DateTime dateTime = {year, month, day, hh, mm, ss};
  return dateTime;
}

/*
void ds1302_writetime(struct DateTime *dt) {
  uint8_t temp;
  SPI.beginTransaction(SPISettings(_maxClock, LSBFIRST, SPI_MODE0));
  digitalWrite(_pin, HIGH);
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
  digitalWrite(_pin, LOW);
  SPI.endTransaction();
}

void print_time(struct DateTime *dt) {
  char tmp[20];
  snprintf(tmp, 20, "%02d/%02d/%02d %02d:%02d:%02d", dt->year, dt->month, dt->day, dt->hour, dt->minute, dt->second);
  Serial.print(tmp);
}
*/

uint8_t RTCDriver::daysPerMonth(uint8_t month, uint8_t year) {
  switch (month) {
    case 4:
    case 6:
    case 9:
    case 11:
      return 30;
    case 2:
      return ((year % 4) == 0) ? 29 : 28;
    default:
      return 31;
  }
}

bool RTCDriver::isValid(struct DateTime* dateTime) {
  if (dateTime->year > 99
      || dateTime->month > 12 || dateTime->month < 1
      || dateTime->hour > 23
      || dateTime->minute > 59
      || dateTime->second > 59)
    return false;

  uint8_t maxDays = RTCDriver::daysPerMonth(dateTime->month, dateTime->year);
  return dateTime->day <= maxDays;
}

