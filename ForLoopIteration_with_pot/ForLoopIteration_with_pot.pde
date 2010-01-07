/*
  For Loop Iteration
 
 Demonstrates the use of a for() loop. 
 Lights multiple LEDs in sequence, then in reverse.
 
 The circuit:
 * LEDs from pins 2 through 7 to ground
 
 created 2006
 by David A. Mellis
 modified 5 Jul 2009
 by Tom Igoe 
 
 http://www.arduino.cc/en/Tutorial/ForLoop
 */

int timer = 100;           // The higher the number, the slower the timing.
int sensorPin = 0;    // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor

void setup() {
  // use a for loop to initialize each pin as an output:
  for (int thisPin = 2; thisPin < 12; thisPin++)  {
    pinMode(thisPin, OUTPUT);      
  }
}

void loop() {
  // read the value from the sensor:
  // loop from the lowest pin to the highest:
  for (int thisPin = 2; thisPin < 12; thisPin++) { 
    // turn the pin on:
    digitalWrite(thisPin, HIGH);
    sensorValue = analogRead(sensorPin); 
    delay(sensorValue);                  
    // turn the pin off:
    digitalWrite(thisPin, LOW);    
  }

  // loop from the highest pin to the lowest:
  for (int thisPin = 10; thisPin >= 3; thisPin--) { 
    // turn the pin on:
    digitalWrite(thisPin, HIGH);
    sensorValue = analogRead(sensorPin); 
    delay(sensorValue);
    // turn the pin off:
    if (thisPin != 2)
      digitalWrite(thisPin, LOW);
  }
}
