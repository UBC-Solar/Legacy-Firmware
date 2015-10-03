/*  UBCSolar current sensor
    Q(unit=C) = fnInt(I(dimensionless),t(unit=ms),lastTime,thisTime) * 5.0(V)/1024*50(A/V)/1000000(us/s)
    
    CONNECTION INFO
    TX: MEGA Serial3 RX
    RX: MEGA Serial3 TX
*/

#include <math.h>
#define SCALECORRECTION 1/1.05
#define BIASCORRECTION 1

//RESISTOR CALIBRATION
#define R1 9940
#define R2 9730
#define R3 9770
#define R4 9900
#define R5 9910

int sensorPin = A0;    // select the input pin for the potentiometer
long int I = 0;
long int Q = 0;
long int Qbias;
unsigned long int lastTime = 0;
unsigned long int thisTime = 0;
unsigned long int lastSendTime = 0;
float resistor[6] = {0,R1,R2,R3,R4,R5};

void setup() {
  Serial.begin(115200);
}

void loop() {
  I=analogRead(sensorPin)-511;
  thisTime=micros();   
  Q=Q+I*(thisTime-lastTime);
  lastTime=thisTime;
  
  if(Serial.available() && Serial.read()=='Q'){ /* TODO condition to send data */
    Qbias = (lastTime-lastSendTime)*BIASCORRECTION;
    
    Serial.print("<");
//    Serial.print(I*5.0/1024*50); /* instant current */
    Serial.print((Q-Qbias)*5.0/1024*50/(lastTime-lastSendTime)*SCALECORRECTION);/* avg current */ /* do all constants at the end */
    Serial.print(",");
    Serial.print((Q-Qbias)*5.0/1024*50/1000000*SCALECORRECTION); /* charge */
    Serial.print(",");

    lastSendTime = lastTime;
    Q=0;
    
    for(int i=1;i<6;i++){
      thermistor(i);
    }
    Serial.println(">");    
  }
}

void thermistor(int pin){
  int v=analogRead(pin);
  float r=resistor[pin]*v/(1023-v);
  float t=1/(0.001125308852122+0.000234711863267*log(r)+0.000000085663516*log(r)*log(r)*log(r)) - 273;
  
  Serial.print(t);
  Serial.print(",");
}
