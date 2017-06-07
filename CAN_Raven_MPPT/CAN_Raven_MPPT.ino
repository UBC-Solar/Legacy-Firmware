/* Current Sensor, Relay, and Array Temperature Sensor Code
 *  UBC Solar
 *  Purpose: 1. To read the current output of the panels on the MPPT, determine whether the batteries are fully charged, and disconnect the panels from the MPPT if necessary
 *           2. Measure the temperature of each of the six arrays 
 *           3. Periodically (every second) send status message containing current levels and temperatures (201, length = 6 bytes)
 *           4. Recieves control message to connect or disconnect solar panels (200, length = 1 byte)
 *           
 *           
 *  Last Update: 06/04/2017
 *  Board: Arduino Mega 2560
 *  Analog Pins: A0 to A11
 *  Digital Pins: 
 *  
 *  Output: 1. Current read, I_p +/-20mA, of each of the 6 current sensor
 *          2. Each MPPT relay status 
 *          3. Each temperature sensor status
 *          4. Battery charge status
 *          
 *  Input:  1. Temperature sensor voltage
 *          2. Current sensor voltage
 *          3. Driver controlled kill switch (enitrely mechanical)
 *          4.
 * 
 * 
 * ID       SYSTEM            LENGTH    FORMAT
 * 
 * 200      RELAY CONTROL       1        data[0-5] = relay [0-5] status (0 = OFF, 1 = ON)
 * 201      SENSOR INFO         8
 * 199      WARNING TEMP
 * 198      WARNING CURRENT
 */

 

/* HEADER FILES */
#include <ubcsolar_can_ids.h>
#include <SPI.h>
#include <mcp_can.h>
#include <math.h>


/* CAN IDS */
#define CAN_ID_MPPT_SENSORS 201
#define CAN_ID_MPPT_CONTROL 200 
#define CAN_ID_MPPT_TEMP_WARNING  
#define CAN_ID_MPPT_CURRENT_WARNING
#define MAX_CURRENT  8000 
#define BUS_SPEED CAN_125KBPS

/* CURRENT SENSOR CONSTANTS */
const float ZERO_CURRENT_VOLTAGE[6] = {2.543,2.539,2.491,2.536,2.541,900000}; 
const float CURRENT_CONVERSION_FACTOR[6] = {.69,.07,.0685,.069, 10000,1000};   // Conversion factor from current sensed to voltage read on analog pins (see ACS712T datasheet under x30A table)

/* TEMPERATURE SENSOR CONSTANTS */
const float TEMP_BASE_VOLTAGE [6] = {603,1000,1000,1000,1000,1000};  // mV
const float TEMP_CONVERSION_FACTOR [6] = {1000,1000,1000,1000,1000,1000};

/* PINS */
const int panel[6] = {A0,A1,A2,A3,A4,A5};     // Analog input pin that the current sensor for the panel input of MPPT 0 is attached to
const int openRelay[6] = {0,1,2,3,4,5};       // Digital output pins corresponding to their respective power relays
const int temp_sensor[6] = {A6,A7,A8,A9,A10,A11};
 
/* CURRENT SENSORS CONGIFURATION*/
float currentValue[6] = {0};        // value read from analog pins A0-A2
float currentVoltageOut[6] = {0};          // Value of voltage read from VIOUT (pin 7 on current sensor and pin A0 on arduino)
float currentOut[6] = {0};          // Value of current through panels


/* TEMPERATURE SENSORS CONFIGURATION */
float tempValue[6] = {0};
float tempOut[6] = {0};
float tempvoltageOut[6] = {0};
float tempCelsius[6] = {0}
float tempF[6] = {0};


/*****************************************************************************************************************************************/

/* CAN Setup */

byte frame_data_current[6];
byte frame_data_temperature[10];

const int SPI_CS_PIN = 9;
MCP_CAN CAN(SPI_CS_PIN);      // Set CS pin

void setup() {
  
  // Initialize serial communications at 9600 bps:
  Serial.begin(115200); 

  for (int i = 0; i < 6; i++) {
    pinMode(panel[i], INPUT);
    pinMode(openRelay[i], OUTPUT);
    pinMode(temp_sensor,INPUT);
  }

  // Initialize CAN bus serial communications at 115200 bps
    int canSSOffset = 0;

 CAN_INIT: 

  if(CAN_OK == CAN.begin(BUS_SPEED)) {
        Serial.println("CAN BUS Shield initilization okay");
    }
    
    else {
        Serial.println("CAN BUS Shield init fail");
        Serial.print("Init CAN BUS Shield again with SS pin ");
        Serial.println(SPI_CS_PIN + canSSOffset);
        delay(100);
        canSSOffset ^= 1;
        CAN = MCP_CAN(SPI_CS_PIN + canSSOffset);
        goto CAN_INIT;
    }
    
  for (int k = 0; k < 6; k++) {
    digitalWrite(openRelay[k], HIGH);  // Output 1 = Relay closed 
  }
} 


void loop() {



  /*For loop reads one current sensor and its corresponding relay at a time*/
  for( int i = 0; i < 6; i++ ){
    /* ADD OFFSET */
    // Reading and displaying the current read
    currentValue[i] = analogRead(panel[i]);
    currentVoltageOut[i] = float((currentValue[i] / 100.0) - 2.667) - ZERO_CURRENT_VOLTAGE[i];   // Convert value read in sensorValue to the voltage value output of the current sensor - 2.543
    currentOut[i] = currentVoltageOut[i] / CURRENT_CONVERSION_FACTOR[i] * 1000 / 2.0;           // Convert voltage value to current value, I_p, read from the current sensor
    
    // Print results to serial monitor
    Serial.print("MPPT");
    Serial.print(i);
    Serial.print(": Voltage (V) = ");
    Serial.print(currentVoltageOut[i]);
    Serial.print("\t Current (mA) = ");
    Serial.print(currentOut[0]);
    Serial.print("\n\r");
    
    // Relay will open if current is larger than the maximum threshold 
    if( currentOut[i] > MAX_CURRENT ){
 
      // Disconnecting panels from MPPT
      digitalWrite(openRelay[i], LOW);
      
      // Notify user of MPPT disconnection
      Serial.print( "\nMPPT" );
      Serial.print( i );
      Serial.print( " disconnected.\n\n" );
    }

    else{
      digitalWrite(openRelay[i], HIGH);
      Serial.print( "\nMPPT" );
      Serial.print( i );
      Serial.print( " connected.\n\n" );
    }
  }

  /* Reading and displaying temperature sensors and sending information through CAN */


  /*
   *float tempValue[6] = {0};
   *float tempOut[6] = {0};
   *float tempvoltageOut[6] = {0};
   */
  
  for (int j = 0; j > 6; j++) {
    float baseVoltage[j] = (TEMP_BASE_VOLTAGE[j]*5.05)/1024.0;   // At room temperature: Convert reading to in (V) -- around 2.94
    float baseCelsius[j] = (baseVoltage[j]/0.01) - 273.15;       // At room temperature: Convert to celsius
    float tempVoltage[j] = analogRead(temp_sensor[j]);           // mV   
    float tempVoltage[j] = (tempVoltage[j]*5.05)/1024;           // V
    float tempConvert[j] = (newTempVoltage[j]/0.01) - 273.15;       
    float tempCelsius[j] = baseCelsius[j] + (baseCelsius[j] - tempConvert[j]);
    float tempF[j] = tempCelsius[j]*(9.0/5.0) + 32.0;
  
    Serial.println("Voltage reading (mV): \n");
    Serial.println(tempVoltage);
    Serial.println(" \n Convert factor (C) : \n");
    Serial.println(tempConvert);
    Serial.println("\n Temperature reading(C): \n");
    Serial.println(tempCelsius);
    Serial.println("\n Temperature reading (F): \n");
    Serial.println(tempF);  
  }
  delay(1000);
}


/* MESSAGE HANDLER
 * Send status of relays, current sensors and temperature sensors
 */

void msgHandler(unsigned char frame_id,unsigned char* frame_data, unsigned char frame_length) {
    
    Serial.println("------------------------------------------");
    Serial.print("data received from ID: ");
    Serial.println(frame_id);
    Serial.print("Frame Data:  ");
        for(int i = 0; i<frame_length; i++)    // print the data
        {
            Serial.print(frame_data[i]);
            Serial.print("\t");
        }
        
     if (frame_id == CAN_ID_MPPT_CONTROL) {

      
/* Sending data through CAN */

for(int x = 0; x < 6; x++) {
  frame_data_current[x] = currentOut[i]; }

for(int y = 0; y< 10; y++) {
  frame_data_temperature[y] = tempCels
}
  

      
     
     }
}

