
#include <mcp_can.h>
#include <SPI.h>

#define PRESSURE_PIN A1
#define ROTATION_PIN A2
#define LED_PIN 8 //FIGURE OUT SENSOR AND LED PINS
#define ID 0x00
#define PRESSURE_THRESHOLD 100
#define ROTATION_THRESHOLD 100
#define TIME_THRESHOLD 100

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 10;


MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
    pinMode(ROTATION_PIN, INPUT);
    pinMode(PRESSURE_PIN, INPUT);    //is this line necessary???
    pinMode(LED_PIN, OUTPUT);
    Serial.begin(115200);

START_INIT:

    if(CAN_OK == CAN.begin(CAN_125KBPS))                   // init can bus : baudrate = 500k
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

unsigned char stmp[1] = {0};
unsigned long previousMillis = 0;

void loop()
{
    unsigned long currentMillis = millis();

    if(currentMillis - previousMillis > TIME_THRESHOLD){
      
      if((analogRead(PRESSURE_PIN) > PRESSURE_THRESHOLD || analogRead(ROTATION_PIN) > ROTATION_THRESHOLD) && stmp[0] == 0){  
        // send data:  id = 0x00, standrad frame, data len = 1, stmp: data buf
        stmp[0] = 1;
        CAN.sendMsgBuf(ID, 0, 1, stmp);
        digitalWrite(LED_PIN, HIGH);

      }else if((analogRead(PRESSURE_PIN) < PRESSURE_THRESHOLD && analogRead(ROTATION_PIN) < ROTATION_THRESHOLD) && stmp[0] == 1){
        /*
        Lockout will only send all-clear message if both sensors don't detect anything; may need to change later depending on performance of sensors
        */ 
        stmp[0] = 0;
        CAN.sendMsgBuf(ID, 0, 1, stmp);
        digitalWrite(LED_PIN, LOW);  
      }

      previousMillis = currentMillis;
   }  
}
