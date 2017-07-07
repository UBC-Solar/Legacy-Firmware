#include <mcp_can.h>
#include <mcp_can_dfs.h>

#include <ubcsolar_can_ids.h>
#include <SPI.h>

#define PRESSURE_PIN0 0
#define PRESSURE_PIN1 1

#define BUS_SPEED CAN_125KBPS
#define CAN_SS 9

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;


MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
    pinMode(PRESSURE_PIN0, INPUT);
    pinMode(PRESSURE_PIN1, INPUT); 
    Serial.begin(115200);
    int canSSOffset = 0;

CAN_INIT:

if(CAN_OK == CAN.begin(BUS_SPEED))                   // init can bus : baudrate = 125k
  {
    Serial.println("CAN BUS Shield init ok!");
  }
  else
  {
    Serial.println("CAN BUS Shield init fail");
    Serial.print("Init CAN BUS Shield again with SS pin ");
    Serial.println(CAN_SS + canSSOffset);
    delay(100);
    canSSOffset ^= 1;
    CAN = MCP_CAN(CAN_SS + canSSOffset);
    goto CAN_INIT;
  }
}

unsigned char stmp[1] = {1};

void loop()
{
  bool pressure0 = digitalRead(PRESSURE_PIN0);
  bool pressure1 = digitalRead(PRESSURE_PIN1);

  if ((pressure0 || pressure1) && stmp[0] == 0) {
    // send data:  id = 0x00, standrad frame, data len = 1, stmp: data buf
    stmp[0] = 1;
    CAN.sendMsgBuf(CAN_ID_BRAKE, 0, 1, stmp);
  } else if (!pressure0 && !pressure1 && stmp[0] == 1) {
    //Lockout will only send all-clear message if both sensors don't detect anything; may need to change later depending on performance of sensors
    stmp[0] = 0;
    CAN.sendMsgBuf(CAN_ID_BRAKE, 0, 1, stmp);
  }  
}
