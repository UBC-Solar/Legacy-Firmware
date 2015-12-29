/*
 *  This code is for receiving messages regarding indicator lights and 
 *  taking action accordingly.
 *  
 *  We have 7 lights on the car
 *  2 front, 2 front sides, 2 back, and 1 back center up
 *  
 *  We need 5 outputs to control all the lights
 *  
 *  1 for front and side, both in the right side
 *  1 for front and side, both in the left side
 *  1 for back left
 *  1 for back right
 *  1 for back center
 *  
 *  ID(dec) SYSTEM              LENGHT  FORMAT
 *  0        brake                1     data[0] = brake status (0=OFF, 1=ON)
 *  
 *  1       Emergency Hazard      1     data[0] = hazard status(0=OFF, 1=ON)
 *  
 *  9       signals               1     data[0].0 = left turning signal status (0=OFF, 1=ON)
 *                                      data[0].1 = right turning signal status (0=OFF, 1=ON)
 *  
 * 
 */

 #include <SPI.h>
#include <mcp_can.h>
#include <ubcsolar_can_ids.h>

// SPI_CS_PIN should be 10 for our version of arduino and shield.    it is 9 in newer versions.
const int SPI_CS_PIN = 9;
MCP_CAN CAN(SPI_CS_PIN); 

#define TRUE 1
#define FALSE 0

// 5 groups of outputs for 7 lights, output pins can be changed according to pins available
#define FRONT_SIDE_R_PIN   8
#define FRONT_SIDE_L_PIN   10

//used for determining if the LED should blink or not.    ledBLINK = 0 => NOT BLINK  ledBLINK = 1 => BLINK
boolean ledBLINK_R=0;        // right side lights
boolean ledBLINK_L=0;        // left side lights
boolean ledBLINK_ALL=0;      // all 4 lights

int ledState_FR =LOW;
int ledState_FL =LOW;


#define HAZARD_INTERVAL 125
#define NORMAL_INTERVAL 300

unsigned long previousMillis =0;
long interval = NORMAL_INTERVAL;       

void setup() {
  
    Serial.begin(115200);
    pinMode(FRONT_SIDE_R_PIN,OUTPUT);
    pinMode(FRONT_SIDE_L_PIN,OUTPUT);

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
    
    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data is coming
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

        if (canID == 0x01)  //Emergency Hazard message
        {
            if (buf[0] == 1)  //Emergency Hazard ON
            {
                ledBLINK_ALL = TRUE;
                Serial.println("leds should start blinking. Emergency Hazard!!!!" );
                
                interval = HAZARD_INTERVAL;   // to make the light blink faster
            }
                
            else if (buf[0] == 0)  //Emergency Hazard OF
            {
                ledBLINK_ALL = FALSE;
                Serial.println("leds should stop blinking. Emergency Hazard is over!!" );

                interval = NORMAL_INTERVAL;   // to make the light blink with normal intervals
            }
        }
        
        else if (canID == CAN_ID_SIGNAL_CTRL)   // Turning Indicator message
        {
            if (buf[0] == 1)  //Turning left side Indicators ON
            {
                ledBLINK_L = TRUE;
                Serial.println("LEFT side lights should start blinking. Turning Indicator ON" );
            }
            else if (buf[0] == 2) //Turning right side Indicators ON
            {
                ledBLINK_R = TRUE;
                Serial.println("RIGHT side lights should start blinking. Turning Indicator ON" );
            }                    
            else if (buf[0] == 0) //Turning Indicators OFF
            {
                ledBLINK_R = FALSE;
                ledBLINK_L = FALSE;
                Serial.println("led should stop blinking. Turning Indicator OFF" );
            }
        }
        
    }

    unsigned long currentMillis = millis();

    if ( currentMillis - previousMillis >= interval)    // to blink the amber light. instead of using delay();
    {          
        previousMillis = currentMillis;

            if (ledBLINK_ALL) //hazard
            {
               ledState_FR =!ledState_FR;
               Serial.print("Front Right---" );
               Serial.print(ledState_FR);
               
               ledState_FL =!ledState_FL;
               Serial.print("--Front Left---" );
               Serial.println(ledState_FL);
            }
            else if(ledBLINK_R)
            {
               ledState_FR =!ledState_FR;
               Serial.print("Front Right---" );
               Serial.print(ledState_FR);
               
               ledState_FL =LOW;
               Serial.print("--Front Left---" );
               Serial.println(ledState_FL);
            }
            else if(ledBLINK_L)
            {
               ledState_FR = LOW;
               Serial.print("Front Right---" );
               Serial.print(ledState_FR);
               
               ledState_FL =!ledState_FL;
               Serial.print("--Front Left---" );
               Serial.println(ledState_FL);
            }

            else
            {
              ledState_FR = LOW;
               Serial.print("Front Right---" );
               Serial.print(ledState_FR);
               
               ledState_FL =LOW;
               Serial.print("--Front Left---" );
               Serial.println(ledState_FL);
            }

            digitalWrite(FRONT_SIDE_R_PIN, ledState_FR);
            digitalWrite(FRONT_SIDE_L_PIN, ledState_FL);
    }
 //   else  // if the last message was to turn the light off
 //   {
 //        ledState_FR = LOW;
  //       ledState_FL = LOW;
  //  }
 //   digitalWrite(FRONT_SIDE_R_PIN, ledState_FR);
  //  digitalWrite(FRONT_SIDE_L_PIN, ledState_FL);
}
