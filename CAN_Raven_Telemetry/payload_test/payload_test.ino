
// demo: CAN-BUS Shield, send data
#include <mcp_can.h>
#include <SPI.h>
#include <ubcsolar_can_ids.h>

#define CAN_ID_HORN 7


// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;
int bytes = 0;
bool idd = false;
int id;
byte stmp[8];
      
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


void loop()
{
      char c;
      

      if (Serial.available()) {
        if (!idd) {
          id = int(Serial.read());
          idd = true;
        }
        else {
           stmp[bytes] = Serial.read();
           bytes++;
        }
        
      }
      if (bytes == 8) {
        bytes = 0;
        idd = false;
        for (int i = 0; i < 8; i++) {
          Serial.print(String(stmp[i]));
          Serial.print(" ");
        }
        Serial.println();
        CAN.sendMsgBuf(id , 0, 8, stmp);
      }
      
}


