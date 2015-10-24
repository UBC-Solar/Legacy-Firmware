#include <SPI.h>
#include "mcp_can.h"

// SPI_CS_PIN should be 10 for our version of arduino and shield.    it is 9 in newer versions.
const int SPI_CS_PIN = 10;
const int LEDpin=7;       // the RED SHIELD has 2 free LEDs on top of it D7 and D8.
boolean ledON=0;        //used for determining if the LED should blink or not.    ledON=0-NOT BLINK  ledON=1- BLINK
MCP_CAN CAN(SPI_CS_PIN); 

unsigned long previousMillis =0;
const long interval = 1000;
int ledState= LOW;          //


void setup() {
  
    Serial.begin(115200);
    pinMode(LEDpin,OUTPUT);

START_INIT:

    if(CAN_OK == CAN.begin(CAN_125KBPS))                   // init can bus : baudrate = 125k
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

void loop() {

    unsigned char len=0, buf[8], canID;

     if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        canID = CAN.getCanId(); 
        
        Serial.println("------------------------------------------");
        Serial.print("get data from ID: ");
        Serial.println(canID);

        Serial.print("Frame Data:  ");
        for(int i = 0; i<len; i++)    // print the data
        {
            Serial.print(buf[i]);
            Serial.print("\t");
        }

        if (canID == 0x00)  //to start blinking
        {
            ledON=1;
            Serial.println("led should start blinking" );
        }

        else if (canID == 0x01)   // to turn the amber light off
        {
            ledON=0;
            Serial.println("led should stop blinking" );
        }
    }

    unsigned long currentMillis = millis();

    if (ledON)  // if the last message was to blink the amber light
    {
      //Serial.print("led STATE IS :      " );
      //Serial.println(ledState);
      
          if ( currentMillis - previousMillis >= interval)    // to blink the amber light. instead of using delay();
          {
            previousMillis = currentMillis;
      
            if (ledState == LOW)
                ledState =HIGH;
            else
                ledState =LOW;
      
            digitalWrite(LEDpin, ledState);
          }
    }
    else  // if the last message was to turn the amber light off
    {
        ledState = LOW;
        digitalWrite(LEDpin, ledState);
    }
}
