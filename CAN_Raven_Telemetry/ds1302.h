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

byte ds1302_io(byte addr, byte data);

void ds1302_init(void);

void ds1302_readtime(struct datetime *dt);

void ds1302_writetime(struct datetime *dt);

void print_time(struct datetime *dt);

int validate_time(struct datetime *dt);

#endif
