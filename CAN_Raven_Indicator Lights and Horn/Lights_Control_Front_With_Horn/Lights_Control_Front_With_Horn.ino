

/*
 *  This code is for receiving messages regarding indicator lights and 
 *  taking action accordingly.
 *  
 *  We have 7 lights on the car
 *  2 front, 2 front sides, 2 back, and 1 behind driver's seat
 *  
 *  We need 5 outputs to control all the lights
 *  
 *  1 for front and side, both in the right side
 *  1 for front and side, both in the left side
 *  1 for back left
 *  1 for back right
 *  1 for back center
 *  
 *  ID(dec) SYSTEM              LENGTH  FORMAT
 *  0        brake                1     data[0] = brake status (0=OFF, 1=ON)
 *  
 *  1       Hazard                1     data[0] = hazard status(0=OFF, 1=ON)
 *  
 *  9       signals               1     data[0].0 = left turning signal status (0=OFF, 1=ON)
 *                                      data[0].1 = right turning signal status (0=OFF, 1=ON)
 *  
 *  ?       Horn                  1     data[0] = horn status (0=OFF, 1=ON)
 */

 #include <SPI.h>
#include <mcp_can.h>
#include <ubcsolar_can_ids.h>

// SPI_CS_PIN should be 10 for older version of shield. It is 9 for the newer version.
const int SPI_CS_PIN = 9;
MCP_CAN CAN(SPI_CS_PIN); 

#define TRUE 1
#define FALSE 0

// 2 arduino pins are used for horn control to split the current required to turn on the Relay  
#define HORN_PIN 4 
#define CAN_ID_HORN 7

// 2 groups of outputs for 4 lights, output pins can be changed according to pins available
#define FRONT_SIDE_R_PIN   6
#define FRONT_SIDE_L_PIN   7

// 3 flages which are actually the message recieved
boolean Hazard=0;
boolean Left_Sig=0;
boolean Right_Sig=0;

/*used to determine if the LED should blink or not. (value determined according to the 4 flags)
 *ledBLINK = 0 => NOT BLINK  ledBLINK = 1 => BLINK
 */
boolean ledBLINK_R=0;        // right side lights
boolean ledBLINK_L=0;        // left side lights
boolean ledBLINK_ALL=0;      // all 4 lights

// LED states for the lights. 2 outputs
int ledState_FR =LOW;
int ledState_FL =LOW;

// time intervals used for blinking
#define HAZARD_INTERVAL 125
#define NORMAL_INTERVAL 300

unsigned long previousMillis =0;
long interval = NORMAL_INTERVAL;       

void setup() {
  
// SERIAL INIT 
    Serial.begin(9600);

// CAN INIT 
    int canSSOffset = 0;
    
    pinMode(FRONT_SIDE_R_PIN,OUTPUT);
    pinMode(FRONT_SIDE_L_PIN,OUTPUT);

    pinMode(HORN_PIN, OUTPUT);

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

void loop() {

    uint32_t canID;
    byte buf[8];
    byte len;
    
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
    
        
        if (canID == CAN_ID_HEARTBEAT){   // Turning Indicator message
          byte signalStatus = buf[3];
          int leftSignalStatus = bitRead(signalStatus, 0);
          int rightSignalStatus = bitRead(signalStatus, 1);
          int hornStatus = bitRead(signalStatus, 3);
          int hazardStatus = bitRead(signalStatus, 4);
  
          if(hazardStatus == 1)            
          {
              Hazard = TRUE;
              Serial.println("leds should start blinking. Emergency Hazard!!!!" );
              
              interval = HAZARD_INTERVAL;   // to make the light blink faster
          } else {
              Hazard = FALSE;
              Serial.println("leds should stop blinking. Emergency Hazard is over!!" );
  
              interval = NORMAL_INTERVAL;   // to make the light blink with normal intervals
          }
  
          if (leftSignalStatus == 1)  //Turning left side Indicators ON
          {
              Left_Sig = TRUE;
              Serial.println("LEFT side lights should start blinking. Turning Indicator ON" );
          } else {
              Left_Sig = FALSE;
          }
          
          if (rightSignalStatus == 1) //Turning right side Indicators ON
          {
              Right_Sig = TRUE;
              Serial.println("RIGHT side lights should start blinking. Turning Indicator ON" );
          } else {
              Right_Sig = FALSE;
          }    
          if (hornStatus == 1)
          {
            Serial.println("Horn is off");
            digitalWrite(HORN_PIN, LOW);
          }
          else
          {
            Serial.println("Horn is on");
            digitalWrite(HORN_PIN, HIGH);
          }
      }
    
    }

    // determining the conditions according to flags
    ledBLINK_ALL = Hazard;
    ledBLINK_R = Hazard || Right_Sig;               
    ledBLINK_L = Hazard || Left_Sig;

// to turn off all the lights which should NOT BLINK or be ON
    
    if ( !ledBLINK_R )
        ledState_FR = LOW;
        
    if ( !ledBLINK_L )
        ledState_FL = LOW;

    
    unsigned long currentMillis = millis();

    if (ledBLINK_R || ledBLINK_L || ledBLINK_ALL)
        
    {           
            if ( currentMillis - previousMillis >= interval)    // to blink the amber light. instead of using delay();
            {
                previousMillis = currentMillis;

                if (ledBLINK_ALL) //hazard
                {
                   ledState_FR =!ledState_FR;
                   Serial.print("Front Right---" );
                   Serial.print(ledState_FR);
                   
                   ledState_FL =ledState_FR;
                   Serial.print("--Front Left---" );
                   Serial.print(ledState_FL);
                }
                else if(ledBLINK_R)
                {
                   ledState_FR =!ledState_FR;
                   Serial.print("Front Right---" );
                   Serial.print(ledState_FR);
                }
                else if(ledBLINK_L)
                {                                 
                   ledState_FL =!ledState_FL;
                   Serial.print("Front Left---" );
                   Serial.print(ledState_FL);
                }
            }

    }
    digitalWrite(FRONT_SIDE_R_PIN, ledState_FR);
    digitalWrite(FRONT_SIDE_L_PIN, ledState_FL);
}
