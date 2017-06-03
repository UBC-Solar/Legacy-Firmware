#include <mcp_can.h>
#include <mcp_can_dfs.h>
#include <SPI.h>
#include <avr/pgmspace.h>

#include "bms_defs.h"
//#include "ds1302.h"
#include <ubcsolar_can_ids.h>

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

void msgHandler(uint32_t frame_id, byte *frame_data, byte length);

void setup() {  
/* SERIAL INIT */
  Serial.begin(19200);

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

/* RTC INIT */
  // must go after CAN init, or need to call SPI.begin()
  SPI.begin();
//  ds1302_init();

/* SD INIT */
  // must go after RTC init
//  log_init();

  Serial.println("System initialized");
}

void loop() {
  byte length;
  uint32_t frame_id;
  byte frame_data[8];

  if(CAN_MSGAVAIL == CAN.checkReceive()){
    CAN.readMsgBuf(&length, frame_data);
    frame_id = CAN.getCanId();
    
    msgHandler(frame_id, frame_data, length);
  }
}


