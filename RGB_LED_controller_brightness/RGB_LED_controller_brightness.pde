/* 
RGB LED controller
4 modes: off, color select, color pulse and random cycle/pulse
By Markus Ulfberg 2009-05-19
Modified by Martin "DarkFox" Eberhardt 2009-12-29

Thanks to: Ladyada, Tom Igoe and 
everyone at the Arduino forum for excellent 
tutorials and everyday help. 

*/

// set the ledPins
int ledRed = 10;
int ledGreen = 9;
int ledBlue = 6;

// color selector pin
int potPin = 3;
int pwrPotPin = 1;

// lightMode selector
int switchPin = 2;

// light mode variable
// initial value 0 = off
int lightMode = 0;

// LED Power variables
byte redPwr = 0;
byte greenPwr = 0;
byte bluePwr = 0;

// Variables for lightMode 3
// set the initial random colors
byte redNew = random(255);
byte greenNew = random(255);
byte blueNew = random(255);


// misc interface variables 
// potVal keeps the value of the potentiometer
int colorVal;
int powerVal;

long previousMillis = 0;        // will store last time LED was updated
long interval = 20;           // interval at which to blink (milliseconds)

int pulseState = 255; // Save where the pulse is. Starting high.
int pulseDir = 0;     // Pulse direction

int buttonState = LOW;         // current state of the button
int lastButtonState = LOW;     // previous state of the button

void setup()
{
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledBlue, OUTPUT);

  pinMode(potPin, INPUT);

  pinMode(switchPin, INPUT);

  // serial for debugging purposes only
  Serial.begin(9600);
}

void loop()
{
  // read the potentiometer position
  colorVal = analogRead(potPin);
  powerVal = map(analogRead(pwrPotPin), 0, 1023, 0, 510);
  
  Serial.println(powerVal, DEC);
  
  buttonState = digitalRead(switchPin);  // read input value and store it in val

  if (buttonState != lastButtonState) {
    if (buttonState == HIGH) {                // check if the button is pressed
      switch(lightMode) {
        case 0:
        lightMode = 1;
        break;
        case 1:
        lightMode = 2;
        break;
        case 2:
        lightMode = 3;
        break;
        case 3:
        lightMode = 0;
        break;
      }
    }
    lastButtonState = buttonState;
  }

  if (lightMode == 0) {      // turn light off
    analogWrite(ledRed, 0);
    analogWrite(ledGreen, 0);
    analogWrite(ledBlue, 0); 
  }
  if (lightMode == 1) {        // set fixed color
    setColor();
    setPower();

    writeLED();
  }

  if (lightMode == 2) {     // pulse fixed color
    if (millis() - previousMillis > interval) {
      // save the last time you blinked the LED 
      previousMillis = millis();
      setColor();
      
      /*
      if (pulseState > powerVal) {
        pulseState = powerVal;
      }
      */
      
      if (pulseDir == 0) {
        pulseState--;
        
        if (pulseState == 0) {
          pulseDir = 1;
        }
      } else {
        pulseState++;
        
        if (pulseState >= 255) {
          pulseDir = 0;
        }
      }
      
      redPwr = map(redPwr, 0, 255, 0, pulseState);
      bluePwr = map(bluePwr, 0, 255, 0, pulseState);
      greenPwr = map(greenPwr, 0, 255, 0, pulseState);
      
      
      writeLED();
    }
  }

  if (lightMode == 3) {  // randomsize colorNew and step colorPwr to it
    if (millis() - previousMillis > interval) {
      // save the last time you blinked the LED 
      previousMillis = millis();
      
      if (redPwr > redNew) {
        redPwr--;
      } 
      if (redPwr < redNew) {
        redPwr++;
      }
      if (greenPwr > greenNew) {
        greenPwr--;
      } 
      if (greenPwr < greenNew) {
        greenPwr++;
      }
      if (bluePwr > blueNew) {
        bluePwr--;
      } 
      if (bluePwr < blueNew) {
        bluePwr++;
      }

      // If all Pwr match New get new colors

      if (redPwr == redNew) {
        if (greenPwr == greenNew) {
          if (bluePwr == blueNew) {
            redNew = random(254);
            greenNew = random(254);
            blueNew = random(254);
          }
        }
      }
      
      writeLED();
    }
  }
}

void setColor() {
    // RED > ORANGE > YELLOW
  if (colorVal >= 0 && colorVal < 170) {
    redPwr = 255;
    bluePwr = 0;
    greenPwr = map(colorVal, 0, 170, 0, 255);
  }

    // YELLOW > LIME?? > GREEN 
  if (colorVal >= 170 && colorVal < 341) {
    greenPwr = 255;
    bluePwr = 0;
    redPwr = map(colorVal, 341, 170, 0, 255);
  }

    // GREEN > TURQOUISE
  if (colorVal >= 341 && colorVal < 511) {
    greenPwr = 255;
    redPwr = 0;
    bluePwr = map(colorVal, 341, 511, 0, 255);
  }

    // TURQOUISE > BLUE  
  if (colorVal >= 511 && colorVal < 682) {
    bluePwr = 255;
    redPwr = 0;
    greenPwr = map(colorVal, 682, 511, 0, 255);
  }

    // BLUE > PURPLE 
  if (colorVal >= 682 && colorVal < 852) {
    bluePwr = 255;
    greenPwr = 0;
    redPwr = map(colorVal, 682, 852, 0, 255);
  }

    // PURPLE > RED
  if (colorVal >= 852 && colorVal <= 1023) {
    redPwr = 255;
    greenPwr = 0;
    bluePwr = map(colorVal, 1023, 852, 0, 255);
  }
  
  if (colorVal == 1024) {
    redPwr = 255;
    greenPwr = 255;
    bluePwr = 255;
  }
}

void setPower() {
  if (powerVal <= 255) {
    redPwr = map(redPwr, 0, 255, 0, powerVal);
    greenPwr = map(greenPwr, 0, 255, 0, powerVal);
    bluePwr = map(bluePwr, 0, 255, 0, powerVal);
  } else {
    int lowVal = powerVal-255;
    redPwr = map(redPwr, 0, 255, lowVal, 255);
    greenPwr = map(greenPwr, 0, 255, lowVal, 255);
    bluePwr = map(bluePwr, 0, 255, lowVal, 255);
  }
}

void writeLED() {
  // display the colors
  analogWrite(ledRed, redPwr);
  analogWrite(ledGreen, greenPwr);
  analogWrite(ledBlue, bluePwr);
}
