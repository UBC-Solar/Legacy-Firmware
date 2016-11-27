
// demo: CAN-BUS Shield, send data
#include <mcp_can.h>
#include <SPI.h>
#include <ubcsolar_can_ids.h>

#define CAN_ID_HORN 7


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

unsigned char stmp[1] = {0};
void loop()
{
      char c;
      
      if(Serial.available())
      {
          c = Serial.read();
          
          if(c == '0')                          // EMERGENCY HAZARD  on
          {
              Serial.println("0");
              Serial.println("Emergency Hazard ---------------------- ON");
              stmp[0]=1;
              CAN.sendMsgBuf(CAN_ID_HAZARD , 0, 1, stmp);
          }
          else if(c == '1')                      // EMERGENCY HAZARD  OFF
          {
              Serial.println("1");
              Serial.println("Emergency Hazard ---------------------- OFF");
              stmp[0]=0;
              CAN.sendMsgBuf(CAN_ID_HAZARD , 0, 1, stmp);
          }
          else if(c == '2')                       // right Indicator  on
          {
              Serial.println("2");
              Serial.println("indicator  ---------------------- ON ------------- RIGHT side");
              stmp[0]=2;
              CAN.sendMsgBuf(CAN_ID_SIGNAL_CTRL , 0, 1, stmp);
          }
          else if(c == '3')                       // left Indicator  on
          {
              Serial.println("3");
              Serial.println("indicator  ---------------------- ON ------------- LEFT side");
              stmp[0]=1;
              CAN.sendMsgBuf(CAN_ID_SIGNAL_CTRL , 0, 1, stmp);
          }
          else if(c == '4')                       // right Indicator  on
          {
              Serial.println("4");
              Serial.println("indicator  ---------------------- OFF  ");
              stmp[0]=0;
              CAN.sendMsgBuf(CAN_ID_SIGNAL_CTRL , 0, 1, stmp);
          }
          else if(c == '5')                       // BRAKE  on
          {
              Serial.println("5");
              Serial.println(" BRAKE  ---------------------- ON ");
              stmp[0]=1;
              CAN.sendMsgBuf(CAN_ID_BRAKE , 0, 1, stmp);
          }
          else if(c == '6')                       // BRAKE  on
          {
              Serial.println("6");
              Serial.println(" BRAKE  ---------------------- OFF");
              stmp[0]=0;
              CAN.sendMsgBuf(CAN_ID_BRAKE , 0, 1, stmp);
          }
          else if(c == '7')
          {
              Serial.println("7");
              Serial.println(" Horn  ---------------------- ON");
              stmp[0]=1;
              CAN.sendMsgBuf(CAN_ID_HORN, 0, 1, stmp);
          }
          else if(c == '8')
          {
              Serial.println("8");
              Serial.println(" Horn  ---------------------- OFF");
              stmp[0]=0;
              CAN.sendMsgBuf(CAN_ID_HORN, 0, 1, stmp);
          }
      }
}
