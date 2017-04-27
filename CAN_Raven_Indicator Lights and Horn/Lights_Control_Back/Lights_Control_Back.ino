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
#define HAZARD_INTERVAL 125
#define NORMAL_INTERVAL 300

// 5 flages for each message
boolean Brake_flg    =0;
boolean Hazard_flg   =0;
boolean Left_Sig_flg =0;
boolean Right_Sig_flg=0;
boolean BPS_Trip_flg =0;

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
long blink_Interval = NORMAL_INTERVAL;       

void setup() {
  
// SERIAL INIT 
    Serial.begin(115200);

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

void msgHandler(unsigned char frame_id,unsigned char* frame_data, unsigned char frame_length)
{
    // ******************* Printing the recieved CAN msg *********************
    Serial.println("------------------------------------------");
    Serial.print("data received from ID: ");
    Serial.println(frame_id);
    Serial.print("Frame Data:  ");
        for(int i = 0; i<frame_length; i++)    // print the data
        {
            Serial.print(frame_data[i]);
            Serial.print("\t");
        }
    // ******************* setting flags based on CAN msg *********************   
    if (frame_id == CAN_ID_HAZARD)  //Emergency Hazard message
    {
        if (frame_data[0] == 1)  //Emergency Hazard ON
        {
            Hazard_flg = TRUE;
            Serial.println("leds should start blinking. Emergency Hazard!!!!" );
            blink_Interval = HAZARD_INTERVAL;   // to make the light blink faster
        }     
        else if (frame_data[0] == 0)  //Emergency Hazard OF
        {
            Hazard_flg = FALSE;
            Serial.println("leds should stop blinking. Emergency Hazard is over!!" );
            blink_Interval = NORMAL_INTERVAL;   // to make the light blink with normal intervals
        }
    }
    else if (frame_id == CAN_ID_SIGNAL_CTRL)   // Turning Indicator message
    {
        if (frame_data[0] == 1)  //Turning left side Indicators ON
        {
            Left_Sig_flg = TRUE;
            Serial.println("LEFT side lights should start blinking. Turning Indicator ON" );
        }
        else if (frame_data[0] == 2) //Turning right side Indicators ON
        {
            Right_Sig_flg = TRUE;
            Serial.println("RIGHT side lights should start blinking. Turning Indicator ON" );
        }                    
        else if (frame_data[0] == 0) //Turning Indicators OFF
        {
            Right_Sig_flg = FALSE;
            Left_Sig_flg = FALSE;
            Serial.println("led should stop blinking. Turning Indicator OFF" );
        }
    }
    else if (frame_id == CAN_ID_BRAKE)  // Brake message
    {
        if (frame_data[0] == 1)  // Brake ON
        {
            Brake_flg = TRUE;
            Serial.println("led should turn on. Brakes ON" );
        }
        else if (frame_data[0] == 0) // Brake OFF
        {
            Brake_flg = FALSE;
            Serial.println("led should turn off. Brakes OFF" );
        }
    }
    else if (frame_id == CAN_ID_ZEVA_BMS_CORE_STATUS)  //BPS Trip Message  TODO for msg ID
    {
        unsigned char error = frame_data[0] & 15; //error is the bits 3-0 of frame_data[0]
        if (error == 3 || error == 5 || error == 8) // BPS Trip Indicator ON      TODO for message data
        {
            BPS_Trip_flg = TRUE;
            Serial.print("White Strobe Light Should turn on. BPS TRIP happened,  ERROR : " );
            if (error == 3)
                Serial.println(" ---- OverCurrent " );
            else if (error == 5)
                Serial.println(" ---- UnderVoltage for +10s " );
            else if (error == 8)
                Serial.println(" ---- OverTemperature " );               
        }
        else if(error == 0) //  No BMS ERROR. BPS Trip Indicator OFF
        {
            BPS_Trip_flg = FALSE;
            Serial.println("White Strobe Light Should turn off. No BMS Error" );
        }
    }
}

void loop() {
  
    unsigned char len=0, buf[8], canID;
    
    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data is coming
    {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
        canID = CAN.getCanId(); 

        msgHandler (canID, buf, len);
    }

    // ******************* setting controller flags *********************
    ledBLINK_ALL = Hazard_flg;
    ledBLINK_R = Hazard_flg || Right_Sig_flg;               
    ledBLINK_L = Hazard_flg || Left_Sig_flg;

    Brake_R= !ledBLINK_R  && Brake_flg;    // since blinking is in priority, Brake_x value is determined if the light is not to be blinked
    Brake_L= !ledBLINK_L  && Brake_flg;      
    Brake_C= !ledBLINK_ALL  && Brake_flg ;

    // ******************* Output 0ing if necessary *********************
    // to turn off all the lights which should NOT BLINK or be ON
    if ( !Brake_R && !ledBLINK_R) //right light
        ledState_BR=LOW;

    if ( !Brake_L && !ledBLINK_L) //left light
        ledState_BL=LOW;
    
    if ( !Brake_C && !ledBLINK_ALL)//center light
        ledState_BC=LOW;

    // ******************* Blinking Control *********************
    unsigned long currentMillis = millis();

    if (ledBLINK_R || ledBLINK_L || ledBLINK_ALL)
    {           
        if ( currentMillis - previousMillis >= blink_Interval)    // to blink the amber light. instead of using delay();
        {
            previousMillis = currentMillis;

            ledState_BC = (ledBLINK_ALL ? !ledState_BC : ledState_BC);
            ledState_BR = (ledBLINK_ALL ?  ledState_BC : (ledBLINK_R ? !ledState_BR : ledState_BR));
            ledState_BL = (ledBLINK_ALL ?  ledState_BC : (ledBLINK_L ? !ledState_BL : ledState_BL));
            /*(ledBLINK_ALL || ledBLINK_R)?.... is not used because it can't guarantee synced blink for all lights
                example:
                  1. hazard On.
                  2. right turn on.
                  3. hazard off.
                  4. hazard on when the right light is on.
                  5. center and left light blink and turn on, while right light blinks and turn off (out of phase)
            */
            /*
            if (ledBLINK_ALL) //hazard    //******** this method , lights are synced ******* too long
            {
                ledState_BR =!ledState_BR;
                Serial.print("--Back Right---" );
                Serial.print(ledState_BR);
                   
                ledState_BL =ledState_BR;
                Serial.print("--Back left---" );
                Serial.print(ledState_BL);
                   
                ledState_BC =ledState_BR;
                Serial.print("--Back Center---" );
                Serial.println(ledState_BC);            
            }
            else if(ledBLINK_R)
            {
                ledState_BR =!ledState_BR;
                Serial.print("--Back Right----" );
                Serial.println(ledState_BR);       
            }
            else if(ledBLINK_L)
            {                                 
                ledState_BL =!ledState_BL;
                Serial.print("--Back Left----" );
                Serial.println(ledState_BL);         
            }*/
        }
    }

    // ******************* Constant ON/OFF Control *********************
    // turning on the lights if they should be on ( braking and not blinking)
    if ( Brake_R )
        ledState_BR = HIGH;
    
    if ( Brake_L )
        ledState_BL = HIGH;

    if ( Brake_C )
        ledState_BC = HIGH;

    if ( BPS_Trip_flg )
        ledState_BPS = HIGH;
    else
        ledState_BPS = LOW;
    
    // ******************* Driving the outputs *********************
    digitalWrite(BACK_R_PIN, ledState_BR);
    digitalWrite(BACK_L_PIN, ledState_BL);
    digitalWrite(BACK_C_PIN, ledState_BC);
    digitalWrite(BPS_PIN, ledState_BPS);
}
