//ZEVA PROTOCOL AT http://zeva.com.au/Products/datasheets/BMS12_CAN_Protocol.pdf

#include <mcp_can.h>
#include <SPI.h>
#include <avr/pgmspace.h>

#include "bms_defs.h"
#include "ds1302.h"
#include <ubcsolar_can_ids.h>

#define DEBUG 0 //print contents of all packets received instead of once per second

#define CAN_SS 10

#define BUS_SPEED CAN_125KBPS
#define PRINT_DELAY 1000
#define DIAG_PRINT_DELAY 2000

#define BINMSG_SEPARATOR 0xFF
#define BINMSG_MAXVAL 0xFE

typedef struct DataPacket{
  byte speed;
  byte voltage;
  byte soc;
  byte bmsTemperature;
  byte motorTemperature;
  byte batteryTemperature[4];
  byte cellVoltageX50[4][12];
} DataPacket;

typedef enum{
  DIAG_OFF,
  DIAG_BMS_CORE,
  DIAG_BMS_CELLS,
  DIAG_OTHERS,
  DIAG_SETTIME
} DiagMode;

MCP_CAN CAN(CAN_SS);

BMSConfig bmsConfig = {0}; //set valid to 0
BMSStatus bmsStatus = {0};
int cellVoltagesX100[4][12] = {{0}};
signed char bmsTemperatures[4][2] = {{0}};

unsigned char bmsAlive = 0;
unsigned long int lastPrintTime = 0;

byte target_throttle = 0;
byte target_regen = 0;
byte target_dir = 0;
unsigned long int lastMotorCtrlRxTime = 0;
byte brake_on = 0;

DiagMode diagnosticMode = DIAG_OFF;

DataPacket dataPacket;

void setup() {  
/* SERIAL INIT */
  Serial.begin(19200);

/* CAN INIT */
  int canSSOffset = 0;

CAN_INIT:

  if(CAN_OK == CAN.begin(BUS_SPEED))                   // init can bus : baudrate = 125k
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
  ds1302_init();

/* SD INIT */
  // must go after RTC init
  log_init();

  Serial.println("System initialized");
}

void printBuf(uint32_t frame_id, byte *frame_data, byte length) {
  Serial.print(F("[Rx] ID: "));
  Serial.print(frame_id,HEX);
        
  Serial.print(F(" Len:"));
  Serial.print(length,HEX);
      
  Serial.print(F(" Data:["));
  for (int i=0;i<length;i++) {
    if (i>0) Serial.print(F(" "));
    Serial.print(frame_data[i],HEX);
  }
  Serial.println(F("]")); 
}

void msgHandleZevaBms(uint32_t frame_id, byte *frame_data, byte length) {
  
  uint32_t messageID = frame_id%10;
  
  // even IDs are requests from Core to BMS12
  if(messageID%2 == 0) {
    #if DEBUG
    switch (messageID) {
      case 0: Serial.print(F("Request for status from Core to Module ")); break;
      case 2: Serial.print(F("Request for voltages #1 from Core to Module ")); break;
      case 4: Serial.print(F("Request for voltages #2 from Core to Module ")); break;
      case 6: Serial.print(F("Request for config info from Core to Module ")); break;
      case 8: Serial.print(F("Command to set config info from Core to Module ")); break;
      default: Serial.print(F("Unknown message from Core to Module ")); break;
    }
    Serial.print((frame_id - 100 - messageID)/10);  // frameID = 100 + 10 * moduleID + messageID, 0 <= messageID <=8
    Serial.println();
    #endif
    return;
  }
  
  if(messageID != 3 && messageID != 5){
    Serial.print(F("BMS #"));
    Serial.print((frame_id-100)/10);
    Serial.print(F(" packet "));
    Serial.print(frame_id%10);
    Serial.println(F(" parsing not implemented"));
    return;
  }
  
  byte bmsId=(frame_id-100)/10;
  byte voltGrp=(frame_id%10)/2-1;
  
  for(int i=0; i<6; i++) {
    cellVoltagesX100[bmsId][voltGrp*6+i] = frame_data[i]+((frame_data[6]>>i)&1 ? 256 : 0);
  }
  bmsTemperatures[bmsId][voltGrp] = frame_data[7] - 128;
  
  #if DEBUG
  Serial.print(F("BMS #"));
  Serial.print(bmsId);
  for(int i=0; i<6; i++) {
    Serial.print(F(" c"));
    Serial.print(i+6*voltGrp);
    Serial.print(F("="));
    Serial.print(cellVoltagesX100[bmsId][voltGrp*6+i]);
  }
  Serial.print(F(" t"));
  Serial.print(voltGrp);
  Serial.print(F("="));
  Serial.print(bmsTemperatures[bmsId][voltGrp]);
  Serial.println();
  #endif
}

void msgHandleMotorCtrl(uint32_t frame_id, byte *frame_data, byte length){
  target_throttle = frame_data[0];
  target_regen = frame_data[1];
  target_dir = frame_data[2];
  lastMotorCtrlRxTime = millis();
}

void msgHandleBrake(uint32_t frame_id, byte *frame_data, byte length){
  brake_on = frame_data[0];
}

void msgHandler(uint32_t frame_id, byte *frame_data, byte length) {
   
  if(frame_id>=CAN_ID_ZEVA_BMS_BASE && frame_id<CAN_ID_ZEVA_BMS_BASE+40) {
    msgHandleZevaBms(frame_id, frame_data, length);
  }
  else if(frame_id == CAN_ID_ZEVA_BMS_CORE_STATUS) {
    msgHandleZevaCoreStatus(frame_id, frame_data, length);
  }
  else if(frame_id >= 12 && frame_id <= 17) {
    msgHandleZevaCoreConfig(frame_id, frame_data, length);
  }
  else if(frame_id == CAN_ID_MOTOR_CTRL){
    msgHandleMotorCtrl(frame_id, frame_data, length);
  }
  else if(frame_id == CAN_ID_BRAKE){
    msgHandleBrake(frame_id, frame_data, length);
  }
  else {
    #if !DEBUG
    if(diagnosticMode != DIAG_OFF){
      diag_cursorPosition(19,1);
    #endif
    Serial.print(F("unknown msg "));
    printBuf(frame_id, frame_data, length);
    #if !DEBUG
    }
    #endif
  }
}

static PROGMEM const uint32_t crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

uint32_t crc32_update(uint32_t crc, byte *data, int len)
{
    byte tbl_idx;
    while(len--){
      tbl_idx = crc ^ (*data >> (0 * 4));
      crc = pgm_read_dword_near(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
      tbl_idx = crc ^ (*data >> (1 * 4));
      crc = pgm_read_dword_near(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
      data++;
    }
    return crc;
}

void printBinMsg(){
  unsigned char tmp;
  uint32_t crc = ~0L;

  dataPacket.speed = 0;
  dataPacket.voltage = (bmsStatus.voltage < 0 ? 0 : (bmsStatus.voltage > BINMSG_MAXVAL ? BINMSG_MAXVAL : bmsStatus.voltage));
  dataPacket.soc = (bmsStatus.soc < 0 ? 0 : (bmsStatus.soc > BINMSG_MAXVAL ? BINMSG_MAXVAL : bmsStatus.soc));
  dataPacket.bmsTemperature = (bmsStatus.temperature < 0 ? 0 : (bmsStatus.temperature > BINMSG_MAXVAL ? BINMSG_MAXVAL : bmsStatus.temperature));
  dataPacket.motorTemperature = 1; //TODO: motor temp

  for(int i=0; i<4; i++){
    dataPacket.batteryTemperature[i] = (bmsTemperatures[i][0] < 0 ? 0 : (bmsTemperatures[i][0] > BINMSG_MAXVAL ? BINMSG_MAXVAL : bmsTemperatures[i][0]));
  }

  for(int i=0; i<4; i++){
    for(int j=0; j<12; j++){
      tmp = cellVoltagesX100[i][j]/2;
      dataPacket.cellVoltageX50[i][j] = (tmp < 0 ? 0 : (tmp > BINMSG_MAXVAL ? BINMSG_MAXVAL : tmp));
    }
  }

  crc = ~0L;
  crc = crc32_update(crc, (byte*) &dataPacket, sizeof(dataPacket));
  crc = ~crc;

  Serial.write(BINMSG_SEPARATOR); //print twice incase one gets lost
  Serial.write(BINMSG_SEPARATOR);
  
  Serial.write((unsigned char *)&dataPacket, sizeof(dataPacket));
  for(int i=24; i>=0; i-=8){
    tmp = (crc >> i) & 0xff;
    Serial.print(tmp >> 4, HEX);
    Serial.print(tmp & 0xf, HEX);
  }
  
  Serial.write(BINMSG_SEPARATOR);
  Serial.write(BINMSG_SEPARATOR);
}

void loop() {
  byte length;
  uint32_t frame_id;
  byte frame_data[8];

  if(CAN_MSGAVAIL == CAN.checkReceive()){
    CAN.readMsgBuf(&length, frame_data);
    frame_id = CAN.getCanId();

    msgHandler(frame_id, frame_data, length);
    log_can(frame_id, length, frame_data);
  }
  
  if(bmsAlive == 1){
    zevaCoreSetCellNum();
    bmsAlive |= 2;
  }

#if !DEBUG
  if(diagnosticMode == DIAG_SETTIME){
    if(Serial.available())
      diag_setTime(Serial.read());
    return;
  }
    
  if(Serial.available()){
    diag_getCmd(Serial.read());
  }

  if(diagnosticMode == DIAG_OFF){
    if(millis() - lastPrintTime > PRINT_DELAY){
      lastPrintTime += PRINT_DELAY;
      printBinMsg();
    }
  }else{
    if(millis() - lastPrintTime > DIAG_PRINT_DELAY){
      lastPrintTime += DIAG_PRINT_DELAY;
      switch(diagnosticMode){
        case DIAG_BMS_CORE:
          diag_BMSCore();
          break;
        case DIAG_BMS_CELLS:
          diag_BMSCells();
          break;
        case DIAG_OTHERS:
          diag_others();
          break;
        default:
          diagnosticMode = DIAG_OFF;
          break;
      }
    }
  }
#endif
}

