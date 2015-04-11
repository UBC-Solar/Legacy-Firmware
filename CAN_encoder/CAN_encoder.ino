/*  New encoder written to support pseudo multitasking
    To ensure accurate encoder readings, the other tasks should exit quickly and return control to the encoder 
    so the encoder can detect the high and low of each cycle.
*/

#define h_pin 4  // analogi pin for reading the IR sensor
#define TIMEOUT 5000000 //microseconds
#define AVGPRD 5000000.0 //microseconds, make sure to add .0 to the end to force floating point or it will fail

float freq;

void setup() {
  Serial.begin(9600);     // start the serial port
  pinMode(h_pin, INPUT);
  Serial.flush();
}

void loop(){  
  encoder();
}

void encoder(){
  static unsigned long lastTime=micros();
  static unsigned long thisTime;
  static unsigned long period;
  static boolean lastState=0;
  static boolean timedout=false;
  static boolean firstTime=true;
  
  //enable these for serial request/response protocol
  if(Serial.available() && Serial.read()=='s')
    Serial.println(freq);
  
  if(micros()-lastTime > TIMEOUT){
    freq=0;
    timedout=true;
    lastTime=micros();
    //enable these for automatic sending (good for CAN)
    //Serial.println();
    //Serial.println(freq);
  }else{
    if(lastState){
      if(!digitalRead(h_pin)){ //detect falling edge
        if(timedout){
          lastTime=micros();
          timedout=false;
          lastState=false;
          firstTime=true;
        }else{
          thisTime=micros();
          period=thisTime-lastTime;
          if(firstTime || period>AVGPRD){
            freq=1000000.0/period; //instantaneous speed
            firstTime=false;
          }else{
            freq=freq * (1-period/AVGPRD) + (1000000.0/period) * (period/AVGPRD); //average speed readings to reduce fluctuations
          }
          lastTime=thisTime;
          lastState=false;
          //enable these for automatic sending (good for CAN)
          //Serial.println();
          //Serial.println(freq);
        }
      }
    }else{
      lastState=digitalRead(h_pin);
    }
  }  
}
