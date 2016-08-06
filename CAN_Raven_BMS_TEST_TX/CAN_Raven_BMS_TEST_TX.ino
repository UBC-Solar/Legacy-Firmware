#include <mcp_can.h>
#include <SoftwareSerial.h>
#include <SPI.h>

#define CAN_SS 9
#define ZEVA_ID 100
#define BUS_SPEED CAN_125KBPS

MCP_CAN CAN(CAN_SS);

byte length,rx_status,filter,ext;
uint32_t frame_id;
byte frame_data[8];

void setup() {                
/* SERIAL INIT */
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

void printBufTX(byte length, uint32_t frame_id, byte *frame_data, byte ext) {
      Serial.print("[TX]"); 
      
      Serial.print(" Len:");
      Serial.print(length,HEX);
      
      Serial.print(" Frame:");
      Serial.print(frame_id,HEX);

      Serial.print(" EXT?:");
      Serial.print(ext==1,HEX);
       
      Serial.print(" Data:[");
      for (int i=0;i<length;i++) {
        if (i>0) Serial.print(" ");
        Serial.print(frame_data[i],HEX);
      }
      Serial.println("]"); 
}

void fakeCurrentSensorPacket(void){
      static int test = 12345;
      union u_tag {
        byte b[4];
        float fval;
      } u;
      
      u.fval=test++;

      Serial.println(u.fval);
     
      frame_data[0] = u.b[0];
      frame_data[1] = u.b[1];
      frame_data[2] = u.b[2];
      frame_data[3] = u.b[3];
      frame_data[4] = 0x14;
      frame_data[5] = 0xC4;
      frame_data[6] = test;
      frame_data[7] = test>>8;
  
      frame_id = 0x7FF;
      length = 8;
}

void fakeBMSPacket(int bmsId, int voltGrp){
  frame_data[0]=1+20*bmsId+6*(voltGrp-1);
  frame_data[1]=2+20*bmsId+6*(voltGrp-1);
  frame_data[2]=3+20*bmsId+6*(voltGrp-1);
  frame_data[3]=4+20*bmsId+6*(voltGrp-1);
  frame_data[4]=5+20*bmsId+6*(voltGrp-1);
  frame_data[5]=6+20*bmsId+6*(voltGrp-1);
  frame_data[6]=0;
  frame_data[7]=77+128;
  
  frame_id=ZEVA_ID + 10*bmsId + 2*voltGrp + 1;
  length=8;
}

void loop() {
      //fakeCurrentSensorPacket();
      //printBufTX(length, frame_id, frame_data, false); //do not remove this print. the compiler will incorrectly optimize
      //CAN.sendMsgBuf(frame_id, 0, length, frame_data);
      
      for(int i=0; i<4; i++){
        fakeBMSPacket(i,1);
        printBufTX(length, frame_id, frame_data, false); //do not remove this print. the compiler will incorrectly optimize
        CAN.sendMsgBuf(frame_id, 0, length, frame_data);
        fakeBMSPacket(i,2);
        printBufTX(length, frame_id, frame_data, false); //do not remove this print. the compiler will incorrectly optimize
        CAN.sendMsgBuf(frame_id, 0, length, frame_data);
      }
      
      delay(100);
}

