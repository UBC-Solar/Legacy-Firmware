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

#include <mcp_can.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <ubcsolar_can_ids.h>

#define BUS_SPEED CAN_125KBPS
#define CAN_SS 9

MCP_CAN CAN(CAN_SS);

byte length;
uint32_t frame_id;
byte frame_data[8];

byte motor_acceleration = 0;
byte current_acceleration = 0;
byte motor_regen;
byte motor_direction;

void setup() {                

  Serial.begin(115200);
  
 /* CAN INIT */
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

void sendMotorControlPacket(void){
  
  frame_id = CAN_ID_HEARTBEAT;
  length = 3;
  
  frame_data[0] = motor_acceleration;
  frame_data[1] = motor_regen;
  frame_data[2] = motor_direction;
  
  CAN.sendMsgBuf(frame_id, 0, length, frame_data);
  
}

void loop() {  
  char c;
  //motorControl();
 
  if(Serial.available())
  {
    c = Serial.read();
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
    //current_acceleration = motor_acceleration;
  }
  
}

