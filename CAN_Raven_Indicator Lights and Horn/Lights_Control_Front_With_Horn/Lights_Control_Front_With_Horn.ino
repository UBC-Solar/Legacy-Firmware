

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
#define HORN_PIN 3

// 2 groups of outputs for 4 lights, output pins can be changed according to pins available
#define FRONT_R_PIN   6
#define FRONT_L_PIN   7

// 3 flages which are actually the message recieved
boolean hazardFlag = LOW;
boolean leftSignalFlag = LOW;
boolean rightSignalFlag = LOW;
boolean hornFlag = LOW;

// LED states for the lights. 2 outputs
int ledState_FR = LOW;
int ledState_FL = LOW;

// time intervals used for blinking
#define HAZARD_INTERVAL 250
#define NORMAL_INTERVAL 500

unsigned long previousMillis;
long blinkInterval = NORMAL_INTERVAL;       

void setup() {
  
// SERIAL INIT 
    Serial.begin(9600);

// CAN INIT 
    int canSSOffset = 0;
    
    pinMode(FRONT_R_PIN,OUTPUT);
    pinMode(FRONT_L_PIN,OUTPUT);

    pinMode(HORN_PIN, OUTPUT);
    digitalWrite(HORN_PIN, LOW);

    previousMillis = millis();

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

void msgHandler(uint32_t frame_id, byte *buf, byte frame_length)
{      
//    Serial.print("frame_id: ");
//    Serial.println(frame_id);
//    Serial.print("buf[0]: ");
//    Serial.println(buf[0]);
//    Serial.print("buf[1]: ");
//    Serial.println(buf[1]);
//    Serial.print("buf[2]: ");
//    Serial.println(buf[2]);
//    Serial.print("buf[3]: ");
//    Serial.println(buf[3], BIN);
  
    if (frame_id == CAN_ID_HEARTBEAT)   // Turning Indicator message
    {
        byte signalStatus = buf[3];
        int leftSignalStatus = bitRead(signalStatus, 0);
        int rightSignalStatus = bitRead(signalStatus, 1);
        int hornStatus = bitRead(signalStatus, 3);
        int hazardSignalStatus = bitRead(signalStatus, 4);

        if (hazardSignalStatus == 1)  //Emergency Hazard ON
        {
            hazardFlag = TRUE;
            Serial.println(F("Hazard true") );
            ledState_FL = HIGH;
            ledState_FR = HIGH;
            blinkInterval = HAZARD_INTERVAL;   // to make the light blink faster
        }     
        else  //Emergency Hazard OFF
        {
            hazardFlag = FALSE;
            Serial.println(F("Hazard false"));
            blinkInterval = NORMAL_INTERVAL;   // to make the light blink with normal intervals
        }
                
        if (leftSignalStatus)  //Turning left side Indicators ON
        {
            leftSignalFlag = TRUE;
            Serial.println(F("left signal true"));
        } else {
            Serial.println(F("left signal false"));
            leftSignalFlag = FALSE;
        }
    
        
        if (rightSignalStatus) //Turning right side Indicators ON
        {
            rightSignalFlag = TRUE;
            Serial.println(F("Right signal true"));
        }                    
        else //Turning Indicators OFF
        {
            rightSignalFlag = FALSE;
            Serial.println(F("Right signal false"));
        }
        if (hornStatus)
        {
            hornFlag = HIGH;
            Serial.println(F("Horn signal true"));
        }
        else {
            hornFlag = LOW;
            Serial.println(F("Horn signal false"));
        }
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

        msgHandler (canID, buf, len);
    }

    unsigned long currentMillis = millis();
    
    if (hazardFlag) {
      if(currentMillis - previousMillis >= blinkInterval){
        previousMillis = currentMillis;

        ledState_FR = !ledState_FR;
        ledState_FL = !ledState_FL;
      }
    } else if (rightSignalFlag || leftSignalFlag) {
      if(currentMillis - previousMillis >= blinkInterval){
        previousMillis = currentMillis;

        if(rightSignalFlag) {
          ledState_FR = !ledState_FR;
          ledState_FL = LOW;
        } else {
          ledState_FR = LOW;
          ledState_FL = !ledState_FL;
        }
      }
    } else {
      ledState_FR = LOW;
      ledState_FL = LOW;
    }
    
    // ******************* Driving the outputs *********************

//      Serial.print("led brake R: ");
//      Serial.println(ledState_FR);
//      Serial.print("led brake L: ");
//      Serial.println(ledState_FL);
//      Serial.print("horn: ");
//      Serial.println(hornFlag);

    digitalWrite(FRONT_R_PIN, ledState_FR);
    digitalWrite(FRONT_L_PIN, ledState_FL);
    digitalWrite(HORN_PIN, hornFlag);
    Serial.print("hornStatus: ");
    Serial.println(hornFlag);
}
