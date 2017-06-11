
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
          else if (c== 'c')
          {
              Serial.println("c");
              Serial.println(" OverCurrent Error from BMS");
              stmp[0]=3 + 2*16; //5th bit is for status = running
              CAN.sendMsgBuf(CAN_ID_ZEVA_BMS_CORE_STATUS, 0, 8, stmp);              
          }
          else if (c== 'v')
          {
              Serial.println("v");
              Serial.println(" UnderVoltage Error from BMS");
              stmp[0]=5 + 2*16;//5th bit is for status = running
              CAN.sendMsgBuf(CAN_ID_ZEVA_BMS_CORE_STATUS, 0, 8, stmp);              
          }
          else if (c== 't')
          {
              Serial.println("t");
              Serial.println(" OverTemperature Error from BMS");
              stmp[0]=8 + 2*16;//5th bit is for status = running
              CAN.sendMsgBuf(CAN_ID_ZEVA_BMS_CORE_STATUS, 0, 8, stmp);              
          }          
          else if (c== 'o')
          {
              Serial.println("o");
              Serial.println(" NO ERROR Error from BMS");
              stmp[0]=0+ 2*16;//5th bit is for status = running
              CAN.sendMsgBuf(CAN_ID_ZEVA_BMS_CORE_STATUS, 0, 8, stmp);              
          }     
          else if (c== 'h')
          {
            Serial.println("h");
            Serial.println(" Heartbeat message from steering wheel");
            stmp[0] = 255;
            stmp[1] = 100;
            stmp[2] = 1;
            bitSet(stmp[3], 4);
            CAN.sendMsgBuf(CAN_ID_HEARTBEAT, 0, 8, stmp);
            for(int i = 0; i < 8; i++){
              bitClear(stmp[3], i);   
            }
          }
          else if (c == 'm')
          {
            Serial.println("m");
            Serial.println(" mppt message");
            unsigned int val1 = 35.23 * 1000;
            unsigned int val2 = 33.21 * 1000;
            unsigned int val3 = 30.97 * 1000;
            unsigned int val4 = 37.21 * 1000;
            stmp[0] = highByte(val1);
            stmp[1] = lowByte(val1);
            stmp[2] = highByte(val2);
            stmp[3] = lowByte(val2);
            stmp[4] = highByte(val3);
            stmp[5] = lowByte(val3);
            stmp[6] = highByte(val4);
            stmp[7] = lowByte(val4);
            CAN.sendMsgBuf(202/*CAN_ID_CURRENT_SENSOR_1*/, 0, 8, stmp);
          }
      }
}
