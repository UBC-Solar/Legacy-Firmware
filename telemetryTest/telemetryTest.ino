/*
 SoftwareSerial example

 Sample of the SoftwareSerial library.  Listens for serial in on pin 2
 and sends it out again on pin 3.

 by Tom Igoe
 based on examples by David Mellis and Heather Dewey-Hagborg
 written: 6 Jan 2007
*/

void setup()  {
 // define pin modes for tx, rx, led pins:
 // set the data rate for the SoftwareSerial port
 Serial.begin(9600);
}

void loop() {
 // listen for new serial coming in:
 // print out the character:
 Serial.println("whatever");
 // toggle an LED just so you see the thing's alive.  
 // this LED will go on with every OTHER character received:

}



