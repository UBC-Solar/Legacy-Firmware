#include <mcp_can.h>
#include <mcp_can_dfs.h>
#include <SPI.h>
#include <SD.h>
#include <avr/pgmspace.h>

#include "bms_defs.h"
#include <ubcsolar_can_ids.h>
#include <virtuabotixRTC.h>

#define CAN_SS 10

#define CAN_SS 10

#define BUS_SPEED CAN_125KBPS
#define PRINT_DELAY 1000
#define DIAG_PRINT_DELAY 2000

#define BINMSG_SEPARATOR 0xFF
#define BINMSG_MAXVAL 0xFE
MCP_CAN CAN(CAN_SS);
byte brake_on = 0;
byte hazard = 0;
float freq;

bool left_signal;
bool right_signal;

struct Motor motor; 
struct BMSCoreStatus bms_status = {0};
struct Battery packs[4] = {0};

String filename;
File logFile;

#define DS1302_SCLK_PIN   7    // Arduino pin for the Serial Clock
#define DS1302_IO_PIN     5    // Arduino pin for the Data I/O
#define DS1302_CE_PIN     6    // Arduino pin for the Chip Enable

// Creation of the Real Time Clock Object

virtuabotixRTC myRTC(DS1302_SCLK_PIN, DS1302_IO_PIN, DS1302_CE_PIN);

void msgHandler(uint32_t frame_id, byte *frame_data, byte length);
void SD_init();
void printONOFF(int input);
void printBMSCoreStatus();
void printBMSCoreError();

void setup() {  
/* SERIAL INIT */
  Serial.begin(9600);

/* CAN INIT */
  int canSSOffset = 0;

CAN_INIT:

  if(CAN_OK == CAN.begin(BUS_SPEED))// init can bus : baudrate = 125k
  {
    Serial.println(F("CAN BUS Shield init ok!"));
  }
  else
  {
    Serial.println(F("CAN BUS Shield init fail"));
    Serial.print(F("Init CAN BUS Shield again with SS pin "));
    canSSOffset ^= 1;
    Serial.println(CAN_SS - canSSOffset);
    delay(100);
    CAN = MCP_CAN(CAN_SS - canSSOffset);
    goto CAN_INIT;
  }

  SD_init();

  Serial.println("System initialized");
}

void loop() {
  myRTC.updateTime();
  byte length;
  uint32_t frame_id;
  byte frame_data[8];

  if(CAN_MSGAVAIL == CAN.checkReceive()){
    CAN.readMsgBuf(&length, frame_data);
    frame_id = CAN.getCanId();
    
    msgHandler(frame_id, frame_data, length);
  }
}

void SD_init() {
  myRTC.updateTime();

  char filenamearr[13];
  int month = myRTC.month;
  int day = myRTC.dayofmonth;
  int hour = myRTC.hours;
  int minute = myRTC.minutes;
  sprintf(filenamearr, "%02u%02u%02u%02u.csv", month, day, hour, minute);

  filename = filenamearr;

  Serial.print("filename: ");
  Serial.println(filename);
  
  Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
}



