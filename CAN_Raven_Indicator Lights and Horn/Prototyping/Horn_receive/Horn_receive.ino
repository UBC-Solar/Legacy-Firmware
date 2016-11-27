#include <SPI.h>
#include <mcp_can.h>
//#include <ubcsolar_can_ids.h>

#define TRUE 1
#define FALSE 0
#define ON 1
#define OFF 0

#define HORN 2 //output pin (can be changed)

const int SPI_CS_PIN = 9;
const int LED=8;
MCP_CAN CAN(SPI_CS_PIN);  

void setup() {
  
  Serial.begin(115200);
  pinMode(LED,OUTPUT);

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

void loop() { //Turns horn on and off

  unsigned char len = 0;
  unsigned char buf[1];
  unsigned char canID;

  if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
  {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        unsigned char canID = CAN.getCanId();
        
        Serial.println("-----------------------------");
        Serial.println("get data from ID: ");
        Serial.println(canID);

        Serial.print("Frame Data:  ");
        
        for(int i = 0; i<len; i++)    // print the data
        {
            Serial.print(buf[i]);
            Serial.print("\t");
        }

        if (canID == 23){             //if horn

            if(buf[0] == 0){
              Serial.println("\nHorn is off");
              digitalWrite(HORN, OFF);
            }
            
            else if(buf[0] == 1){
              Serial.println("\nHorn is on");
              digitalWrite(HORN, ON);
            }
        }
    } 
}
