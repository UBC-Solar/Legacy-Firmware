

/* Current Sensor, Relay, and Array Temperature Sensor Code
 * Last Update: 06/10/2017
 * Board: Arduino Mega 2560
 * 
 * 
 * ID       SYSTEM                           COMPONENT NUMBERS    LENGTH     FORMAT
 *                                           (INCLUSIVE)          (BYTE) 
 *                                                                      
 * 
 * 202      CURRENT SENSORS                  (0 to 3)             8          Each current value is 2 bytes in mA; This ID contains 4 values; First 2 bytes are current sensor #0
 *                                                                         > format_data[0] = highByte(currentsensor[0]) ; format_data[1] = lowByte(currentsensor[0]) 
 *                                                                           format_data[2] = highByte(currentsensor[1]) ; format_data[3] = lowByte(currentsensor[1])
 *                                                                           format_data[4] = highByte(currentsensor[2]) ; format_data[5] = lowByte(currentsensor[2])
 *                                                                           format_data[6] = highByte(currentsensor[3]) ; format_data[7] = lowByte(currentsensor[3])                                                                                      
 * 
 * 201      CURRENT SENSORS                  (4 to 5)             4          Same as above; This ID contains 2 values; First 2 bytes are current sensor #4
 * 
 * 200      (EXTERNAL) RELAY CONTROL         (0 to 5)             6          Receiving this!; (0 = OPEN, 1 = CLOSED); First byte is relay #0            
 *                                                                         > format_data[0] = relay [0] ; format_data[1] = relay [1] ; format_data[2] = relay [2] 
 *                                                                           format_data[3] = relay [3] ; format_data[4] = relay [4] ; format_data[5] = relay [5]  
 *        
 * 199      TEMP. SENSORS                    (0 to 3)             8          Each temp. value is 2 bytes in milliCelsius (lol); This ID contains 4 values; First 2 bytes are temp. sensor #0 
 *                                                                         > format_data[0] = highByte(tempsensor[0]) ; format_data[1] = lowByte(tempsensor[0])
 *                                                                           format_data[2] = highByte(tempsensor[1]) ; format_data[3] = lowByte(tempsensor[1])
 *                                                                           format_data[4] = highByte(tempsensor[2]) ; format_data[5] = lowByte(tempsensor[2])
 *                                                                           format_data[6] = highByte(tempsensor[3]) ; format_data[7] = lowByte(tempsensor[3])
 *                                                                           
 * 198      TEMP. SENSORS                    (4 to 7)             8          Same as above; This ID contains 4 values; First 2 bytes are temp. sensor #4
 * 
 * 197      TEMP. SENSORS                    (8 to 9)             4          Same as above; This ID contains 2 values; First 2 bytes are temp. sensor #8
 * 
 * 196      EXTREME CURRENT WARNING          General              1          Using LSB  -- currently sends a message only when LSB is 1 
 *                                                                         > frame_data[0] = warning_current 
 * 
 * 195      EXTREME TEMP WARNING             General              1          Using LSB  -- currently sends a message only when LSB is 1 
 *                                                                         > frame_data[0] = warning_temp
 * 
 * 194      RELAY STATUS                     (0 to 5)             6          Sending status!; (0 = OPEN, 1 = CLOSED); First byte is relay #0
 *                                                                         > format_data[0] = relay [0] ; format_data[1] = relay [1] ; format_data[2] = relay [2] 
 *                                                                           format_data[3] = relay [3] ; format_data[4] = relay [4] ; format_data[5] = relay [5]  
 * 
  */

#include <ubcsolar_can_ids.h>
#include <SPI.h>
#include <math.h>
#include <mcp_can.h>
#include <mcp_can_dfs.h>

#define CAN_ID_MPPT_CURRENT1 202          //0-3
#define CAN_ID_MPPT_CURRENT2 201          //4-5
#define CAN_ID_MPPT_CONTROL 200           // External control of relay (independent from kil switch
#define CAN_ID_MPPT_TEMP1 199             // 0-3
#define CAN_ID_MPPT_TEMP2 198             // 4-7
#define CAN_ID_MPPT_TEMP3 197             // 8-9 
#define CAN_ID_MPPT_CURRENT_WARNING 196
#define CAN_ID_MPPT_TEMP_WARNING 195     
#define CAN_ID_MPPT_RELAY_STATUS 194  
#define MAX_CURRENT  8000.0
#define MAX_TEMP 80.0                     // Celsius
#define BUS_SPEED CAN_125KBPS
#define VCC 5.05
#define CONNECT 1
#define DISCONNECT 0

// CURRENT SENSOR CONSTANTS
const float ZERO_CURRENT_VOLTAGE[6] = {2.543,2.539,2.491,2.536,2.534,2.537};      // Voltage output from VIOUT/input to A0 at current sensed is 0A (measured)
const float CURRENT_CONVERSION_FACTOR[6] = {.0687,.07,.0685,.069,.0683,.079};     // Conversion factor from current sensed to voltage read on analog pins (see ACS712T datasheet under x30A table)
const float uC_OFFSET[6] = {-2.667,-2.661,-2.619,-2.674,-2.676,-2.663};
float currentValue[6] = {0};
float currentVoltageOut[6] = {0};
float currentOut[6] = {0};
int currentCAN[6] = {0};

// TEMPERATURE SENSOR CONSTANTS
const float TEMP_BASE_VOLTAGE [10] = {620,617,613,607,0,610,615,620,620,619};     // FIX TEMPERATURE SENSOR 5!!!
float baseVoltage[10] = {0}; 
float baseCelsius[10] = {0};
float tempVoltage[10] = {0};
float temp2Voltage[10] = {0};
float tempConvert[10] =  {0};                     
float tempCelsius[10] =  {0};    
float tempF[10] = {0};
int tempCAN[10] = {0};

// PIN CONFIG
const int panel[6] = {A0,A1,A2,A3,A4,A5};                                         // Analog input pin that the current sensor for the panel input of MPPT 0 is attached to
const int relay[6] = {2,3,4,5,6,7};                                               // Digital output pins corresponding to their respective power relays
const int temp_sensor[10] = {A6,A7,A8,A9,A10,A11,A12,A13,A14,A15};
 
// CAN SETUP 
byte frame_data_current1[8] = {0};
byte frame_data_current2[4] = {0};
byte frame_data_temperature1[8] = {0};    // Will be sending in two seperate IDs because there is too much data
byte frame_data_temperature2[8] = {0};
byte frame_data_temperature3[4] = {0};
byte frame_data_relay_status[6] = {0};

byte warning_current = 0;                 // Overcurrent state when LSB is 1
byte warning_temp = 0;                    // Overtemperature state when LSB is 1

const int SPI_CS_PIN = 10;

MCP_CAN CAN(SPI_CS_PIN);

void setup() {
  
// Initialize serial communications at 115200 bps
  Serial.begin(115200);

// Set pin modes and turn relays on
//  for (int i = 0; i < 6; i++) {
//    pinMode(panel[i], INPUT);
//    pinMode(relay[i], OUTPUT);
//    digitalWrite(relay[i], HIGH);
//  }

  // Initialize relays
  digitalWrite(relay[0], 1);
  digitalWrite(relay[1], 1);
  digitalWrite(relay[2], 1);
  digitalWrite(relay[3], 1);
  digitalWrite(relay[4], 1);
  digitalWrite(relay[5], 1);
  
  for (int x = 0; x < 10; x++) {
    pinMode(temp_sensor[x],INPUT);
  }
  
// Initialize CAN bus serial communications
  int canSSOffset = 0;
    
CAN_INIT: 

  if (CAN_OK == CAN.begin(BUS_SPEED)) {
    Serial.println("CAN BUS Shield init okay");
  } else {
        Serial.println("CAN BUS Shield init fail");
        Serial.print("Init CAN BUS Shield again with SS pin ");
        Serial.println(SPI_CS_PIN + canSSOffset);
       
        delay(100);
        
        canSSOffset ^= 1;
        CAN = MCP_CAN(SPI_CS_PIN + canSSOffset);
        goto CAN_INIT;
  }
}

void loop() {

  byte length;
  uint32_t frame_id;
  byte frame_data[8];   // Max length


  // checking for relay control message 
  if(CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBuf(&length, frame_data);
    frame_id = CAN.getCanId();   
    msgHandler(frame_id, frame_data, length);   
  }

  // sending relay status 
  for (int i = 0; i < 5; i++) {
    frame_data_relay_status[i] = relay[i];
  }
  CAN.sendMsgBuf(CAN_ID_MPPT_RELAY_STATUS,0,6,frame_data_relay_status);
  
  // reading and displaying current levels, as well as relay logic
  for (int i = 0; i < 6; i++) {
    currentValue[i] = analogRead(panel[i]);
    currentVoltageOut[i] = float((currentValue[i] / 100.0) + uC_OFFSET[i]) - ZERO_CURRENT_VOLTAGE[i];      // Convert value read in sensorValue to the voltage value output of the current sensor - 2.543
    currentOut[i] = currentVoltageOut[i] / CURRENT_CONVERSION_FACTOR[i] * 1000.0 / 2.0;                    // Convert voltage value to current value, I_p, read from the current sensor
    
    // for CAN
    currentCAN[i] = int (currentOut[i] * 100.0);
   
    Serial.print("MPPT");
    Serial.print(i);
    Serial.print(": Voltage (V) = ");
    Serial.print(currentVoltageOut[i]);
    Serial.print("\t Current (mA) = ");
    Serial.print(currentOut[i]);
    Serial.print("\n\r");

    // relay logic
    if (currentOut[i] >= MAX_CURRENT) {
      // Disconnect
      digitalWrite(relay[i], LOW);
      
      // notify user
      Serial.print( "\nMPPT" );
      Serial.print( i );
      Serial.print( " disconnected.\n\n" );

      bitSet(warning_current,0); // LSB is set to 1
      
    } else {
//      digitalWrite(relay[i], HIGH);
      Serial.print( "\nMPPT" );
      Serial.print( i );
      Serial.print( " connected.\n\n" );

      bitClear(warning_current,0); // LSB is set to 0
    }
  }

// temperature reading and display
  for (int j = 0; j < 10; j++) {
    baseVoltage[j] = (TEMP_BASE_VOLTAGE[j]*VCC)/1024.0;                      // At room temperature: Convert reading to in (V) -- around 2.94
    baseCelsius[j] = (baseVoltage[j]/0.01) - 273.15;                         // At room temperature: Convert to celsius -- around 27
    tempVoltage[j] = analogRead(temp_sensor[j]);                             // Real-time (mV)   
    temp2Voltage[j] = (tempVoltage[j]*VCC)/1024;                             // Real-time (V)
    tempCelsius[j] = ((14*((temp2Voltage[j] / baseVoltage[j]) - 1))+1) * baseCelsius[j] - 23;
    tempF[j] = tempCelsius[j]*(9.0/5.0) + 32.0;                              // Convert to weird American units

   // for CAN
    tempCAN[j] = int (tempCelsius[j]*100.0);

    Serial.print("LM335Z");
    Serial.print(j);
    Serial.print(": Analog Voltage (mV):");
    Serial.print(tempVoltage[j]);
    Serial.print("\t Temperature (C): ");
    Serial.print(tempCelsius[j]);
    Serial.print("\t Temperature (F): ");
    Serial.print(tempF[j]);  
    Serial.print("\n\r");

    // relay logic
    if (tempCelsius[j] > MAX_TEMP) {
      
      // disconnect
      for (int y = 0; y < 6; y++) {
        digitalWrite(relay[y], LOW);
      }
      
      // notify user
      Serial.print( "\nMPPTs" );
      Serial.print(" disconnected.\n\n" );
      bitSet(warning_temp,0);    // LSB set to 1
        
    } else { 
      bitClear(warning_temp, 0);  // LSB set to 0
    }
  }

// Sending data through CAN bus shield  -- Each value is 2 bytes and so multiple IDs are being used to send all the data
// 6 Current Values
// 10 Temperature Values

for ( int x = 0, z = 0; (x < 4) && (z < 8); z+2, x++) {
    frame_data_temperature1[z] = highByte(tempCAN[x]); 
    frame_data_temperature1[z+1] = lowByte(tempCAN[x]);
    Serial.print("\n");
    Serial.print(frame_data_temperature1[z]);
    Serial.print("\n");
  }

for ( int x = 4, z = 0; (x < 8) && (z < 8); z+2, x++) {
    frame_data_temperature2[z] = highByte(tempCAN[x]); 
    frame_data_temperature2[z+1] = lowByte(tempCAN[x]);
    Serial.print("\n");
    Serial.print(frame_data_temperature2[z]);
    Serial.print("\n");
  }

for ( int x = 8, z = 0; (x < 10) && (z < 4); z+2, x++) {
    frame_data_temperature3[z] = highByte(tempCAN[x]); 
    frame_data_temperature3[z+1] = lowByte(tempCAN[x]);
    Serial.print("\n");
    Serial.print(frame_data_temperature3[z]);
    Serial.print("\n");
  }

for ( int x = 0, z = 0; (x < 4) && (z < 8); z+2, x++) {
    frame_data_current1[z] = highByte(currentCAN[x]); 
    frame_data_current1[z+1] = lowByte(currentCAN[x]);
    Serial.print("\n");
    Serial.print(frame_data_current1[z]);
    Serial.print("\n");
  }


for ( int x = 4, z = 0; (x < 6) && (z < 4); z+2, x++) {
    frame_data_current2[z] = highByte(currentCAN[x]); 
    frame_data_current2[z+1] = lowByte(currentCAN[x]);
    Serial.print("\n");
    Serial.print(frame_data_current2[z]);
    Serial.print("\n");
  }
  
  CAN.sendMsgBuf(CAN_ID_MPPT_CURRENT1,0,8,frame_data_current1); 
  CAN.sendMsgBuf(CAN_ID_MPPT_CURRENT2,0,4,frame_data_current2); 
  CAN.sendMsgBuf(CAN_ID_MPPT_TEMP1,0,8,frame_data_temperature1);
  CAN.sendMsgBuf(CAN_ID_MPPT_TEMP2,0,8,frame_data_temperature2);
  CAN.sendMsgBuf(CAN_ID_MPPT_TEMP3,0,4,frame_data_temperature3);

  if (bitRead(warning_current,0) == 1) {
    byte* warning_current_pointer = &warning_current;
    CAN.sendMsgBuf(CAN_ID_MPPT_CURRENT_WARNING, 0 , 1 , warning_current_pointer); 
  }
  
  if (bitRead(warning_temp,0) == 1) {
    byte* warning_temp_pointer = &warning_temp;
  CAN.sendMsgBuf(CAN_ID_MPPT_TEMP_WARNING, 0 , 1 , warning_temp_pointer); } 

 

  delay(1000);
}

// Handling recieved messages from CAN bus shield 
void msgHandler(uint32_t frame_id, byte *frame_data, byte frame_length) {
  
    if (frame_id == CAN_ID_MPPT_CONTROL) {
      for (int i = 0; i < frame_length; i++) {
        digitalWrite(relay[i], frame_data[i] );
      }
    } else {
        Serial.print("unknown message");
      }
}



