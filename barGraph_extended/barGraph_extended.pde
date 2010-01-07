/*
  LED bar graph
 
  Turns on a series of LEDs based on the value of an analog sensor.
  This is a simple way to make a bar graph display. Though this graph
  uses 10 LEDs, you can use any number by changing the LED count
  and the pins in the array.
  
  This method can be used to control any series of digital outputs that
  depends on an analog input.
 
  The circuit:
   * LEDs from pins 2 through 11 to ground
 
 created 26 Jun 2009
 by Tom Igoe 
 
 http://www.arduino.cc/en/Tutorial/BarGraph
 */


// these constants won't change:
const int analogPin = 0;    // the pin that the potentiometer is attached to
const int ledCount = 10;    // the number of LEDs in the bar graph

// Variables will change:
long previousMillis = 0;        // will store last time LED was updated

// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval = 1000;           // interval at which to blink (milliseconds)

int ledPins[] = { 
  2, 3, 4, 5, 6, 7,8,9,10,11 };   // an array of pin numbers to which LEDs are attached
int ledState[] = {};

void setup() {
  // loop over the pin array and set them all to output:
  for (int thisLed = 0; thisLed < ledCount; thisLed++) {
    pinMode(ledPins[thisLed], OUTPUT); 
    ledState[thisLed] = LOW;             // ledState used to set the LED
  }
}

void loop() {
  // read the potentiometer:
  int sensorReading = analogRead(analogPin);
  // map the result to a range from 0 to the number of LEDs:
  int ledLevel = map(sensorReading, 0, 1023, 0, ledCount);

  // loop over the LED array:
  for (int thisLed = 0; thisLed < ledCount; thisLed++) {
    // if the array element's index is less than ledLevel,
    // turn the pin for this element on:
    if (thisLed < ledLevel) {
      if (millis() - previousMillis > interval) {
      // save the last time you blinked the LED 
      previousMillis = millis();   

      // if the LED is off turn it on and vice-versa:
      if (ledState == LOW)
        ledState[thisLed] = HIGH;
      else
        ledState[thisLed] = LOW;
      
        // set the LED with the ledState of the variable:
        digitalWrite(ledPins[thisLed], ledState[thisLed]);
      }
    } 
    // turn off all pins higher than the ledLevel:
    else {
      digitalWrite(ledPins[thisLed], LOW); 
      ledState[thisLed] = LOW;
    }
  }
}



