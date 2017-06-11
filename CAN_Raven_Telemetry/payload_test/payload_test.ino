
// demo: CAN-BUS Shield, send data
#include <mcp_can.h>
#include <SPI.h>
#include <ubcsolar_can_ids.h>

#define CAN_ID_HORN 7


// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;
bool manual = false;

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
      byte stmp[8] = {0x55, 0xA5, 0xAA, 0xFF, 0x86, 0x10, 0x00, 0x00};
      if(Serial.available())
      {
          c = Serial.read();
          int pack = int(c) - 48;
          if (pack >  3) {
            Serial.println("\n\nRunning with manual payload:");
            printPayload(stmp);
            manual = true;
            pack %= 4;
          }
          else {
            manual = false;
            Serial.println("\n\nRunning with random payloads:");
             for (int i = 0; i < 8; i++) {
              stmp[i] = millis()%255;
              delay(10);
            }
          }
          Serial.println("================================================");
          Serial.println("Sending signal 10");
          CAN.sendMsgBuf(10, 0, 8,stmp);
          if (!manual) {
            Serial.println("Payload:");
            printPayload(stmp);
          }
          delay(1000);

          for (int i =0 ; i < 6 ; i++) {
            Serial.println("================================================");
            Serial.println("Sending signal 1" + String(pack) + String(i));
            if (!manual) {
             for (int i = 0; i < 8; i++) {
              stmp[i] = millis()%255;
              delay(10);
               }
              Serial.println("Payload:");
          printPayload(stmp);
            }
            CAN.sendMsgBuf(100 + pack*10 + i, 0, 8, stmp);
           delay(1000);
      
      }
      }
      Serial.flush();
}

void printPayload(byte stmp[]) {
  for (int i = 0; i < 8; i++) {
     for(byte mask = 0x80; mask; mask >>= 1){
   if(mask  & stmp[i])
       Serial.print('1');
   else
       Serial.print('0');
 }
 Serial.println();
}
}

