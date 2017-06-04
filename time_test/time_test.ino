//Daily files can be read via bluetooth by sending MMDD
             
#include <SD.h>
#include <SPI.h>                 // SD
#include <string.h>              //Date As Filename 

#define DS1307_ADDRESS 0x68

char filename[13],charBuf [13],strYr[4];
File myFile, dumpFile;
int  second, minute, hour, weekDay, monthDay, month, year;
const int chipSelect = 4;
String readString;
String stringFive, stringSix;

void setup() {
  Serial.begin(115200);
  Serial.println("Bare test");
  Serial.println("Init SD CARD");
  // make sure that the default chip select pin 53 is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);//Uno 10, MEGA 53
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) 
  {
    Serial.println("Card failed");
    // don't do anything more:011
    return;
  }
  Serial.println("CARD OK");
  delay(2000);
  GetClock();
  Serial.println("today's date for filename");
  Serial.print(year);
  Serial.print("/");
  Serial.print(month);
  Serial.print("/");
  Serial.println(monthDay);

  getFileName();
  Serial.println("active filename");
  Serial.println(filename);
  delay(2000);

 myFile = SD.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // re-open the file for reading:
  myFile = SD.open(filename);
  if (myFile) {
    Serial.println(filename);

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void loop() {

} 
byte bcdToDec(byte val)  {
  // Convert binary coded decimal to normal decimal bers
  return ( (val/16*10) + (val%16) );
}

void getFileName(){
  sprintf(filename, "%04u%02u%02u.csv", 2017, 06, 04);
}

void GetClock(){

}

void getDump() {
  dtostrf(year,4, 0, strYr);
  stringSix = strYr + readString + ".csv";
  stringSix.toCharArray(charBuf, 15);  
   File dumpFile = SD.open(charBuf);
  if (dumpFile) 
  {
    Serial.println("DUMP FROM ");
    Serial.println(charBuf);
     delay(2000);
    while (dumpFile.available())
    {
      Serial.write(dumpFile.read());
    }
    dumpFile.close();
  }  
  else {
    Serial.println("error opening file");
  }
}
