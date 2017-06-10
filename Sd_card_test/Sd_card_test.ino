#include <SPI.h>
#include <SD.h>

#include <virtuabotixRTC.h>

#define DS1302_SCLK_PIN   7    // Arduino pin for the Serial Clock
#define DS1302_IO_PIN     5    // Arduino pin for the Data I/O
#define DS1302_CE_PIN     6    // Arduino pin for the Chip Enable

// Creation of the Real Time Clock Object
//SCLK -> 13, I/O -> 12, CE -> 6
virtuabotixRTC myRTC(DS1302_SCLK_PIN, DS1302_IO_PIN, DS1302_CE_PIN);

File myFile;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
// myRTC.setDS1302Time(0, 17, 12, 1, 4, 6, 2017);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
//
//
  Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  
  String filename = "test2.txt";
  
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
  myRTC.updateTime();
  
  String filename = "";
  filename = filename + myRTC.year;
  filename = filename + "-";
  filename = filename + myRTC.month;
  filename = filename + "-";
  filename = filename + myRTC.dayofmonth;
  filename = filename + "-";
  filename = filename + myRTC.hours;
  filename = filename + "-";
  filename = filename + myRTC.minutes;
  filename = filename + "-";
  filename = filename + myRTC.seconds;
  filename = filename + ".txt";

  Serial.println(filename);

  delay(5000);
}
