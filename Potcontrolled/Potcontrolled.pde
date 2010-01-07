/*
 Fading
 
 This example shows how to fade an LED using the analogWrite() function.
 
 The circuit:
 * LED attached from digital pin 9 to ground.
 
 Created 1 Nov 2008
 By David A. Mellis
 Modified 17 June 2009
 By Tom Igoe
 
 http://arduino.cc/en/Tutorial/Fading
 
 */

int sensorPin = 0;
int sensorValue = 0;  // variable to store the value coming from the sensor
int ledPin = 9;    // LED connected to digital pin 9

void setup()  { 
  // nothing happens in setup 
} 

void loop()  {
  sensorValue = analogRead(sensorPin);
  analogWrite(ledPin, sensorValue/4);
}


