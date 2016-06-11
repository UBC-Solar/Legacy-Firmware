#ifndef DS1302_H
#define DS1302_H

struct datetime {
  byte year; //2 digit year
  byte month;
  byte day;
  byte hour;
  byte minute;
  byte second;
};

#endif