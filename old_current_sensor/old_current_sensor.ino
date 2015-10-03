/*  UBCSolar current sensor
    Q(unit=C) = fnInt(I(dimensionless),t(unit=ms),lastTime,thisTime) * 5.0(V)/1024*50(A/V)/1000000(us/s)
*/

#include <math.h>
#define SCALECORRECTION 1/1.05
#define BIASCORRECTION 1

int sensorPin = A0;    // select the input pin for the potentiometer
long int I = 0;
long int Q = 0;
long int Qbias;
unsigned long int lastTime = 0;
unsigned long int thisTime = 0;
unsigned long int lastSendTime = 0;

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
  }
}
