//Code by Patrick Cruce(pcruce_at_igpp.ucla.edu) 
//Uses CAN extended library, designed for Arduino with 16 Mhz oscillator
//Library found at: http://code.google.com/p/canduino/source/browse/trunk/#trunk%2FLibrary%2FCAN
//This runs a simple test of the hardware with the MCP 2515 CAN Controller in loopback mode.
//If using over physical bus rather than loopback, and you have high bus 
//utilization, you'll want to turn the baud of the serial port up or log
//to SD card, because frame drops can occur due to the reader code being
//occupied with writing to the port.
//In our testing over a 40 foot cable, we didn't have any problems with
//synchronization or frame drops due to SPI,controller, or propagation delays
//even at 1 Megabit.  But we didn't do any tests that required arbitration
//with multiple nodes.
//This code is modified to control the motor by serial monitor of the commanding
//arduino

#include <CAN.h>
#include <SoftwareSerial.h>
#include <SPI.h>

#define BUS_SPEED 125

#define CAN_ID_SIGNAL_CTRL 9
#define CAN_ID_MOTOR_CTRL 4
#define CAN_ID_MPPT_CURRENT 200
#define CAN_ID_UI_BUTTONS 700

#define BIT_SIGNAL_BRAKE 1
#define BIT_SIGNAL_LEFT 2
#define BIT_SIGNAL_RIGHT 4

#define MOTOR_CTRL_INTERVAL 20
#define SIGNAL_CTRL_INTERVAL 500
#define MPPT_CURRENT_INTERVAL 1000

#define MPPT_SENSOR_PIN A0
#define MPPT_SENSOR_SHUNT_R 0.002
#define MPPT_SENSOR_AMP_GAIN 20

byte length,rx_status,filter,ext;
uint32_t frame_id;
byte frame_data[8];

union floatbytes {
  byte b[4];
  float f;
} mppt_i;

byte motor_acceleration = 0;
byte current_acceleration = 0;
byte motor_regen;
byte motor_direction;

byte signal_lights;

unsigned long int lastTX = millis();

void setup() {                

  
  Serial.begin(115200);
  
  // initialize CAN bus class
  // this class initializes SPI communications with MCP2515
  CAN.begin();
  
  CAN.setMode(CONFIGURATION);
  CAN.baudConfig(BUS_SPEED);
 
  CAN.setMode(NORMAL);  // set to "NORMAL" for standard com

  CAN.toggleRxBuffer0Acceptance(true, true);
  CAN.toggleRxBuffer1Acceptance(true, true); 
  CAN.resetFiltersAndMasks();
}


void sendMotorControlPacket(void){
  frame_id = CAN_ID_MOTOR_CTRL;
  length = 3;
  
  frame_data[0] = motor_acceleration;
  frame_data[1] = motor_regen;
  frame_data[2] = motor_direction;
  
  while(CAN.TX0Busy());
  CAN.load_ff_0(length,&frame_id,frame_data, false);
}


//Previous code for motor control. Can not control with serial monitor
//Runs the motor to half the maximum speed, decrease to 0, change direction
//runs the motor again to half the maximum speed.
/*void motorControl(){
  static unsigned long int lastTX = millis();
  static int state = 0;
  static int cntr = 0;
  if(millis() - lastTX > MOTOR_CTRL_INTERVAL){
    switch(state){
      case 0:
        if(motor_acceleration < 128){
          motor_acceleration++;
          Serial.print("Accelerate Forward");
        }else{
          state = 1;
        }
        break;

      case 1:
        if(motor_acceleration > 0){
          motor_acceleration--;
          Serial.print("Accelerate Reverse");
        }else{
          state = 0;
          motor_direction = !motor_direction;
          Serial.print("Change motor direction");
        }
        break;
    }
    

    sendMotorControlPacket();
    lastTX += MOTOR_CTRL_INTERVAL;
    Serial.print("m");
  }
}
*/


void loop() {  
  char c;
  //motorControl();
  static unsigned long int lastTX = millis();
 
  if(Serial.available())
  {
    c = Serial.read();
   if(millis() - lastTX > MOTOR_CTRL_INTERVAL){
    if(c == '0')
    {
      if(motor_acceleration < 256){
          motor_acceleration = motor_acceleration + 8;
          Serial.print("Accelerate Forward, motor_acceleration is: ");
          Serial.print(motor_acceleration);
          Serial.print("\n");
        }
        else{
          Serial.print("Reached Maximum Acceleration\n");
        }

    }
    if(c == '1')
    {
      if(motor_acceleration > 0){
          motor_acceleration = motor_acceleration - 8;
          Serial.print("Accelerate Reverse, motor_acceleration is:");
          Serial.print(motor_acceleration);
          Serial.print("\n");
        }else{
          Serial.print("Reached Minimum Acceleration\n");
        }
    }
    if(c == '2')
    {
      //if(motor_acceleration <= 0)
      //{
      current_acceleration = motor_acceleration;
      motor_acceleration = 0;
      Serial.print("BREAK pressed, motor accleration is: ");
      Serial.print(motor_acceleration);
      Serial.print("\n");
      delay(100);
    }
    if(c == '3')
    {
      motor_direction = !motor_direction;
      motor_acceleration = current_acceleration;
      Serial.print("Change Motor Direction, Motor Direction is: ");
      Serial.print(motor_direction);
      Serial.print("\n");
      Serial.print("Resuming previous acceleration, motor acceleration is: ");
      Serial.print(motor_acceleration);
      Serial.print("\n");
    }    
    sendMotorControlPacket();
    lastTX += MOTOR_CTRL_INTERVAL;
    //current_acceleration = motor_acceleration;
  }
  }
  if(millis() - lastTX > 1000){
    lastTX += 1000;
    Serial.println();
  }
}

