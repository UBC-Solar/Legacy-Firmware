#include <SD.h>

#include <virtuabotixRTC.h>

// Creation of the Real Time Clock Object
//SCLK -> 13, I/O -> 12, CE -> 6
virtuabotixRTC myRTC(7, 5, 6);

void setup() { 
Serial.begin(9600);

// Set the current date, and time in the following format:
// seconds, minutes, hours, day of the week, day of the month, month, year
//myRTC.setDS1302Time(00, 42, 13, 1, 4, 6, 2017);
}


void loop() { 
// This allows for the update of variables for time or accessing the individual elements. 
myRTC.updateTime(); 

// Start printing elements as individuals 
Serial.print("Current Date / Time: "); 
Serial.print(myRTC.dayofmonth); 
Serial.print("/"); 
Serial.print(myRTC.month); 
Serial.print("/");
Serial.print(myRTC.year);
Serial.print(" ");
Serial.print(myRTC.hours);
Serial.print(":");
Serial.print(myRTC.minutes);
Serial.print(":");
Serial.println(myRTC.seconds);
//
//  Serial.print("year: ");
//  Serial.println(myRTC.year);
//  Serial.print("minutes: ");
//  Serial.println(myRTC.minutes);
//  
//  String filename = "";
//  filename = filename + myRTC.year;
//  filename = filename + "-";
//  filename = filename + myRTC.month;
//  filename = filename + "-";
//  filename = filename + myRTC.dayofmonth;
//  filename = filename + ".txt";
//
//  Serial.println(filename);

// Delay so the program doesn't print non-stop
delay( 5000); 
}
