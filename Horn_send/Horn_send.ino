// demo: CAN-BUS Shield, send data
#include <mcp_can.h>
#include <SPI.h>

#define ON 1
#define OFF 0

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 10;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
    Serial.begin(115200);

START_INIT:

    if(CAN_OK == CAN.begin(CAN_500KBPS))                   // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init ok!");
    }
    else
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println("Init CAN BUS Shield again");
        delay(100);
        goto START_INIT;
    }
}

unsigned char stmp[1] = {0};


void loop() {
  char input;
  if(Serial.available()){

    input = Serial.read();
    
   if(input == '1'){
      Serial.println("1");
      Serial.print("On sent");
      stmp[0]=ON;
      CAN.sendMsgBuf(23, 0, 1, stmp);
    }

    else if(input == '0'){
      Serial.println("0");
      Serial.print("Off sent");
      stmp[0]=OFF;
      CAN.sendMsgBuf(23, 0, 1, stmp);
    }
  }
}
