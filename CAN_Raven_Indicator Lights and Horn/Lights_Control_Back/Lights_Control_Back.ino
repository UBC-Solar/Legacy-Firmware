/*
 *  This code is for receiving messages regarding indicator lights and 
 *  taking action accordingly.
 *  
 *  We have 8 lights on the car
 *  2 front, 2 front sides, 2 back, and 1 back center up, 1 whiet strobe light (BPS Trip Indicator)
 *  
 *  We need 6 outputs to control all the lights
 *  
 *  1 for front and side, both in the right side
 *  1 for front and side, both in the left side
 *  1 for back left
 *  1 for back right
 *  1 for back center
 *  1 for whiet strobe light (BPS Trip Indicator)
 *  
 *  ID(dec) SYSTEM              LENGTH(byte)    FORMAT                                                          DEFINED NAME
 *  0        brake                1             data[0] = brake status (0=OFF, 1=ON)                            CAN_ID_BRAKE 
 *  
 *  1       Hazard                1             data[0] = hazard status(0=OFF, 1=ON)                            CAN_ID_HAZARD 
 *  
 *  9       signals               1             data[0].0 = left turning signal status (0=OFF, 1=ON)            CAN_ID_SIGNAL_CTRL 
 *                                              data[0].1 = right turning signal status (0=OFF, 1=ON)
 *  
 *  10      BMS Broadcast Status  8             data[0].3-0 = 3,5,8 (datas related to BPS TRIP INDICATOR)    CAN_ID_ZEVA_BMS_CORE_STATUS 
 *                                              3 = Overcurrent
 *                                              5 = Battery Undervoltage for +10s
 *                                              8 = BMS Overtemperature
 */

#include <SPI.h>
#include <mcp_can.h>
#include <ubcsolar_can_ids.h>

// SPI_CS_PIN should be 10 for our older version of shield. It is 9 for the newer version.
const int SPI_CS_PIN = 9;
MCP_CAN CAN(SPI_CS_PIN); 


//********************************* rear node constants *******************************************
// 4 outputs for 3 lights in the back and 1 BPS Trip Indicator. Pins can be changed
#define BPS_PIN      4
#define BACK_R_PIN   5
#define BACK_C_PIN   6
#define BACK_L_PIN   7

#define TRUE 1
#define FALSE 0

// time intervals used for blinking
#define HAZARD_INTERVAL 250
#define NORMAL_INTERVAL 500
#define BMS_INTERVAL 1000

// 5 flages for each message
boolean brakeFlag    =0;
boolean hazardFlag   =0;
boolean leftSignalFlag =0;
boolean rightSignalFlag=0;
boolean bpsTripFlag =0;

/*used to determine if the LED should blink or not. (value determined according to the 4 flags)
 *ledBLINK = 0 => NOT BLINK  ledBLINK = 1 => BLINK
 */
boolean ledBLINK_R=0;        // right side lights
boolean ledBLINK_L=0;        // left side lights
boolean ledBLINK_ALL=0;      // all 4 lights

// used to determine if the relative light should be on for brake or not.  (value determined according to the 4 flags and ledBLINK_* states)
boolean Brake_R=0;      
boolean Brake_L=0;      
boolean Brake_C=0;      

// LED states for the lights. 3 outputs
boolean ledState_BR =LOW;
boolean ledState_BL =LOW;
boolean ledState_BC =LOW;
boolean ledState_BPS=LOW;

unsigned long previousMillis =0;
long blinkInterval = NORMAL_INTERVAL;
long bmsBlinkInterval = BMS_INTERVAL;       

void setup() {
  
// SERIAL INIT 
    Serial.begin(9600);

// CAN INIT 
    int canSSOffset = 0;
    
    pinMode(BACK_R_PIN,OUTPUT);
    pinMode(BACK_L_PIN,OUTPUT);
    pinMode(BACK_C_PIN,OUTPUT);
    pinMode(BPS_PIN, OUTPUT);

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
        int brakeSignalStatus = bitRead(signalStatus, 2);
        int hazardSignalStatus = bitRead(signalStatus, 4);

        

        if (hazardSignalStatus == 1)  //Emergency Hazard ON
        {
            hazardFlag = TRUE;
            Serial.println(F("Hazard true") );
            ledState_BL = HIGH;
            ledState_BR = HIGH;
            ledState_BC = HIGH;
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
        if (brakeSignalStatus)
        {
            brakeFlag = TRUE;
            Serial.println(F("Brake signal true"));
        }
        else {
            brakeFlag = FALSE;
            Serial.println(F("Brake signal false"));
        }
    }
    else if (frame_id == CAN_ID_BRAKE)  // Brake message
    {
        if (buf[0])  // Brake ON
        {
            brakeFlag = TRUE;
            Serial.println(F("led should turn on. Brakes ON"));
        }
        else if (!buf[0]) // Brake OFF
        {
            brakeFlag = FALSE;
            Serial.println(F("led should turn off. Brakes OFF"));
        }
    }
    else if (frame_id == CAN_ID_AUX_BMS)
    {
        unsigned char error = buf[0];
        if (error == 1) // BPS Trip Indicator ON
        {
            bpsTripFlag = TRUE;
            Serial.print(F("White Strobe Light Should turn on. BPS TRIP happened"));     
        }
        else if(error == 0) //  No BMS ERROR. BPS Trip Indicator OFF
        {
            bpsTripFlag = FALSE;
            Serial.println(F("White Strobe Light Should turn off. No BMS Error"));
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
    if(brakeFlag && !hazardFlag) {
        ledState_BC = HIGH;
        if(!leftSignalFlag){
          ledState_BL = HIGH;
        }
        if(!rightSignalFlag){
          ledState_BR = HIGH;
        }
    }

    if(!brakeFlag && !hazardFlag) {
      ledState_BC = LOW;
    }
    
    if (hazardFlag) {
      if(currentMillis - previousMillis >= blinkInterval){
        previousMillis = currentMillis;

        ledState_BR = !ledState_BR;
        ledState_BL = !ledState_BL;
        ledState_BC = !ledState_BC;
      }
    } else if (rightSignalFlag || leftSignalFlag) {
      if(currentMillis - previousMillis >= blinkInterval){
        previousMillis = currentMillis;

        if(rightSignalFlag) {
          ledState_BR = !ledState_BR;
          ledState_BL = brakeFlag;
        } else {
          ledState_BR = brakeFlag;
          ledState_BL = !ledState_BL;
        }
      }
    } else if (!brakeFlag){
      ledState_BR = LOW;
      ledState_BL = LOW;
      ledState_BC = LOW;
    }
    
    if ( bpsTripFlag ) {
      if(currentMillis - previousMillis >= bmsBlinkInterval) {
        previousMillis = currentMillis;
        ledState_BPS = !ledState_BPS;
      }
    }
    else {
      ledState_BPS = LOW;
    }
      
    // ******************* Driving the outputs *********************
//
//      Serial.print("led brake R: ");
//      Serial.println(ledState_BR);
//      Serial.print("led brake L: ");
//      Serial.println(ledState_BL);
//      Serial.print("led brake C: ");
//      Serial.println(ledState_BC);
//      Serial.print("led brake W: ");
//      Serial.println(ledState_BPS);

    digitalWrite(BACK_R_PIN, ledState_BR);
    digitalWrite(BACK_L_PIN, ledState_BL);
    digitalWrite(BACK_C_PIN, ledState_BC);
    digitalWrite(BPS_PIN, ledState_BPS);
}
