/*
    all the IDs and buf data used in this code are temporary and need to be assigned later
    
    rear light

    Emergency, high priority,             ID 0            buf[0] = 1 => blink,    
                                                          buf[0]= 0 => emergency finished / OFF
                                                          
    turn indicator , moderate priority ,  ID 1            buf[0] = 1 => blink
                                                              buf[1] = 0 => right side blink   && left side off/on if brake signal is on, 
                                                              buf[1] = 1 => left side blink   && right side off/on if brake signal is on,   
                                                          buf[0]= 0 => off,
                                                          
    brake, low priority,                  ID 2            buf[0] = 1 => on,       
                                                          buf[0]= 0 => off
*/

#include <SPI.h>
#include <mcp_can.h>

// SPI_CS_PIN should be 10 for our version of arduino and shield.    it is 9 in newer versions.
const int SPI_CS_PIN = 10;
MCP_CAN CAN(SPI_CS_PIN); 

#define TRUE 1
#define FALSE 0

#define RIGHT 0            // the same as the data in buf[1]
#define LEFT 1             // the same as the data in buf[1]

#define RIGHT_OR_LEFT RIGHT    // we determin if the code is for lights on the right side or left side
// #define RIGHT_OR_LEFT LEFT    .... if the code is for the left side light                               

const int LEDpin=7;       // the RED SHIELD has 2 free LEDs on top of it D7 and D8.
boolean ledBLINK=0;        //used for determining if the LED should blink or not.    ledBLINK = 0 => NOT BLINK  ledBLINK = 1 => BLINK
boolean ledON=0;          //used for determining if the LED should be ON or OFF.     ledON = 0 => OFF       ledON = 1 => ON


unsigned long previousMillis =0;
long interval = 500;
int ledState= LOW;          



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

        if (canID == 0x00)  //Emergency Hazard message
        {
            if (buf[0] == 1)  //Emergency Hazard ON
            {
                ledBLINK = TRUE;
                Serial.println("led should start blinking. Emergency Hazard!!!!" );
                
                interval = 250;   // to make the light blink faster
            }
                
            else if (buf[0] == 0)  //Emergency Hazard OF
            {
                ledBLINK = FALSE;
                Serial.println("led should stop blinking. Emergency Hazard is over!!" );

                interval = 500;   // to make the light blink with normal intervals
            }
        }

        else if (canID == 0x01)   // Turning Indicator message
        {
            if (buf[0] == 1)  //Turning Indicator ON
            {
              if ( buf[1] == RIGHT_OR_LEFT )
              {
                ledBLINK = TRUE;
                Serial.println("led should start blinking. Turning Indicator ON" );
                
                if ( RIGHT_OR_LEFT == RIGHT)                      // just for showing which side is Blinking
                  Serial.println("the Right side is on");
                else
                  Serial.println("the LEFT side is on");
              }
              else   
              {
                ledBLINK = FALSE;
                Serial.println("led should NOT BLINK. Turning Indicator ON for the OTHER SIDE" );
              }
            } 
            else if (buf[0] == 0) //Turning Indicator oOFF
            {
                ledBLINK = FALSE;
                Serial.println("led should stop blinking. Turning Indicator OFF" );
            }
        }

        else if (canID == 0x02)  // Brake message
        {
            if (buf[0] == 1)  // Brake ON
            {
                ledON = TRUE;
                Serial.println("led should turn on. Brakes ON" );
            }
            else if (buf[0] == 0) // Brake OFF
            {
                ledON = FALSE;
                Serial.println("led should turn off. Brakes OFF" );
            }
        }
    }

    unsigned long currentMillis = millis();

// since if else statement is used, if blinking and braking are on together, only blinking, which should have a higher priority, happens

    if (ledBLINK)  // if the last message was to blink the amber light
    {
       Serial.print("led STATE IS :      " );
       Serial.println(ledState);
      
          if ( currentMillis - previousMillis >= interval)    // to blink the amber light. instead of using delay();
          {
            previousMillis = currentMillis;
      
            if (ledState == LOW)
                ledState =HIGH;
            else
                ledState =LOW;
      
  //          digitalWrite(LEDpin, ledState);
          }
    }
    else if (ledON) //if the last message was to turn the light on (brake light)
    {
        ledState = HIGH;
    }
    else  // if the last message was to turn the light off
    {
        ledState = LOW;
    }

    digitalWrite(LEDpin, ledState);
}
