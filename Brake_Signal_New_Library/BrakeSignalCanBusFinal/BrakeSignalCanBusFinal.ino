#include <mcp_can.h>
#include <mcp_can_dfs.h>


//#include <ubcsolar_can_ids.h>
//#include <mcp_can.h>
#include <SPI.h>

#define PRESSURE_PIN A0
#define ROTATION_PIN A1
#define LED_PIN 8 
#define ID 0x00
#define PRESSURE_THRESHOLD 100
#define PRESSURE_DERIVATIVE 75
#define ROTATION_THRESHOLD 17
#define TIME_THRESHOLD 100

#define BUS_SPEED CAN_125KBPS
#define CAN_SS 9

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;


MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
    pinMode(ROTATION_PIN, INPUT);
    pinMode(PRESSURE_PIN, INPUT);    //is this line necessary???
    pinMode(LED_PIN, OUTPUT);
    Serial.begin(115200);
    int canSSOffset = 0;

CAN_INIT:

if(CAN_OK == CAN.begin(BUS_SPEED))                   // init can bus : baudrate = 125k
  {
    Serial.println("CAN BUS Shield init ok!");
  }
  else
  {
    Serial.println("CAN BUS Shield init fail");
    Serial.print("Init CAN BUS Shield again with SS pin ");
    Serial.println(CAN_SS + canSSOffset);
    delay(100);
    canSSOffset ^= 1;
    CAN = MCP_CAN(CAN_SS + canSSOffset);
    goto CAN_INIT;
  }
}

unsigned char stmp[1] = {1};
unsigned long previousMillis = 0;
int pressureValues[15];
int pressureIndex = 0;
bool pressureBraking = false;
int initCount;

void loop()

{
    unsigned long currentMillis = millis();

    if(currentMillis - previousMillis > TIME_THRESHOLD){
      while(initCount < 15){
        pressureValues[initCount] = analogRead(PRESSURE_PIN);
        initCount++;
      }
      
      int currentPressure = analogRead(PRESSURE_PIN);

      int i;
      int arrayMax = pressureValues[0];
      int arrayMin = pressureValues[0];
      for(i = 1; i < 15; i++){
        if(pressureValues[i] > arrayMax){
          arrayMax = pressureValues[i];
        }
        if(pressureValues[i] < arrayMin){
          arrayMin = pressureValues[i];
        }
      }

      if((currentPressure - arrayMin) > PRESSURE_DERIVATIVE){
        pressureBraking = true;
      }
      if(pressureBraking && currentPressure < PRESSURE_THRESHOLD){
        pressureBraking = false;
      }
      
      pressureValues[pressureIndex] = currentPressure;
      if(pressureIndex == 14){
        pressureIndex = 0;
      }else{
        pressureIndex++;
      }

      /*The previous block of code is a rough way to determine the derivative of the pressure sensor's readings.
        The pressure sensor does not have a consistent resting value, so to determine when it is detecting a press of the brake pedal,
        we store the previous 15 readings and compare them to the current reading. If the difference between the minimum of those previous 15 and the current value
        is high enough, we determine that the pedal is pushed in, and send the braking message. There were issues doing the same when releasing the pedal slowly,
        as the derivative would not get low enough (large negative number) if you released the pedal very slowly, so in order to detect a release, we just check
        to see if the value goes below a certain threshold. When the pedal is resting at the same pressure, the reading trends upwards, which is why we have
        to use the derivative to determine when it is pressed, but the reading never trends downwards at constant pressure, so we can use a simple threshold
        to detect the release.*/

         
//      Serial.print(currentPressure);
//      Serial.println(",");
      
      int rotationValue = analogRead(ROTATION_PIN);
      Serial.print(rotationValue);
      Serial.println(",");
      
      if((pressureBraking || rotationValue >= ROTATION_THRESHOLD) && stmp[0] == 0){  
        // send data:  id = 0x00, standrad frame, data len = 1, stmp: data buf
        stmp[0] = 1;
        CAN.sendMsgBuf(ID, 0, 1, stmp);
        digitalWrite(LED_PIN, HIGH);
        Serial.print(700);
        Serial.println(",");

      }else if((!pressureBraking && rotationValue < ROTATION_THRESHOLD) && stmp[0] == 1){
        /*
        Lockout will only send all-clear message if both sensors don't detect anything; may need to change later depending on performance of sensors
        */ 
        stmp[0] = 0;
        CAN.sendMsgBuf(ID, 0, 1, stmp);
        digitalWrite(LED_PIN, LOW);
        Serial.print(-300);
        Serial.println(",");  
      }

      previousMillis = currentMillis;
   }  
}
