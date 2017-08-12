// demo: CAN-BUS Shield, send data
#include <mcp_can.h>
#include <SPI.h>
#include <ubcsolar_can_ids.h>


// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
    Serial.begin(115200);

    int canSSOffset = 0;

START_INIT:

        if(CAN_OK == CAN.begin(CAN_125KBPS))                   // init can bus : baudrate = 125k
    {
        Serial.println("CAN BUS Shield init ok!");
    }
    else
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.print("Init CAN BUS Shield again with SS pin ");
        Serial.println(SPI_CS_PIN + canSSOffset);
        
        delay(100);

        canSSOffset ^= 1;
        CAN = MCP_CAN(SPI_CS_PIN + canSSOffset);
        goto START_INIT;
    }
}

void msgHandler (uint32_t canID, byte buf[], byte len) {
  if (canID == CAN_ID_HEARTBEAT){
    Serial.println("got a heartbeat!");
    Serial.print("accel: ");
    Serial.println(buf[0]);
    Serial.print("regen: ");
    Serial.println(buf[1]);
    Serial.print("direction: ");
    Serial.println(buf[2]);
    Serial.print("signals: ");
    Serial.println(buf[3], BIN);
    Serial.print("brake: ");
    Serial.println(bitRead(buf[3], 2));
  }
  else if(canID == CAN_ID_BRAKE) {
    Serial.println("got a brake message!");
    Serial.print("value: ");
    Serial.println(buf[0]);
  }
  else {
    Serial.println("got unknown message");
    Serial.print("canID: ");
    Serial.println(canID);
  }
  
}

void loop()
{
  
    uint32_t canID;
    byte buf[8]; 
    byte len;
    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data is coming
      {
          CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
          canID = CAN.getCanId(); 

          msgHandler (canID, buf, len);
      }
}

