#include <stdarg.h>
#include <EEPROM.h>
#include <IRremote.h>

/* 
RGB LED IR controller for Arduino
By Martin "DarkFox" Eberhardt 2009-12-29
http://darkfox.dk/

Based on RGB LED controller by Markus Ulfberg 2009-05-19
http://genericnerd.blogspot.com/2009/05/arduino-mood-light-controller.html
*/

// set the ledPins
int ledRed = 9;
int ledGreen = 6;
int ledBlue = 5;

int lastCode;
int code;
int RECV_PIN = 2;
IRrecv irrecv(RECV_PIN);
decode_results results;

// light mode variable
// initial value 0 = off
int lightMode = 0; // Saved in EEPROM address 0

// Variables for saving the last state before standby
int lastLightMode = 1; // EEPROM address 7
int lastPowerVal; // EEPROM address 8+9

// LED Power variables
byte redPwr = 0;
byte greenPwr = 0;
byte bluePwr = 0;

// Variables for colors to fade to
// set the initial random colors
byte redNew = random(255);
byte greenNew = random(255);
byte blueNew = random(255);


// misc interface variables 
// colorVal keeps the value of the potentiometer
int colorVal = 0;   // Color hue (0 - 1023) Saved in EEPROM address 1+2
int powerVal = 255; // Color brightness (0 - 510) Saved in EEPROM address 3+4

long previousDelayMillis = 0; // Will store last time button was pressed
long buttonDelay = 200;        // Delay before repeating button press

long previousMillis = 0;      // will store last time thing was updated
long interval = 50;           // interval at which to blink (milliseconds) (0 - 4095) Saved in EEPROM address 5+6

int pulseState = 255; // Save where the pulse is. Starting high.
int pulseDir = 0;     // Pulse direction

void setup()
{  
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledBlue, OUTPUT);
 
  
  lightMode = EEPROM.read(0);
  
  colorVal = loadBig(1,2);

  powerVal = loadBig(3,4);
  
  if (powerVal == 0) {
    powerVal = 255;
  }
  
  interval = loadBig(5,6);
  
  lastLightMode = EEPROM.read(7);
  
  lastPowerVal = loadBig(8,9);

  //pinMode(potPin, INPUT);

  irrecv.enableIRIn(); // Start the receiver

  // serial
  Serial.begin(9600);
}

void loop()
{

  if (irrecv.decode(&results)) {
    //Serial.println(results.value, HEX);
    irrecv.resume(); // Receive the next value

    if (results.value != 0xFFFFFFFF) {
      code = results.value;
      lastCode = code;
      previousDelayMillis = millis();
    } else {
      if (millis() - previousDelayMillis > buttonDelay) {
        code = lastCode;
      } else {
        code = 0x0;
      }
    }
    
    switch(code) {
      case 0x61D638C7: // FM
      if (results.value != 0xFFFFFFFF) {
        if (lightMode == 3) {
          changeMode(1);
        }
        colorVal = random(1023);
      } else {
        changeMode(3);
      }
      break;
 
      case 0x61D648B7: // Power
      if (results.value != 0xFFFFFFFF) {
        if (lightMode == 0) {
          changeMode(lastLightMode);
        } else {
          changeMode(0);
        }
      }
      break;
 
      case 0x61D6807F: // 1
      turnOn(1);
      colorVal = 0; // Red
      break;
 
      case 0x61D640BF: // 2
      turnOn(1);
      colorVal = 40; // Orange
      break;

      case 0x61D6C03F: // 3
      turnOn(1);
      colorVal = 140; // Yellow
      break;

      case 0x61D620DF: // 4
      turnOn(1);
      colorVal = 250; // Lime
      break;

      case 0x61D6A05F: // 5
      turnOn(1);
      colorVal = 341; // Green
      break;

      case 0x61D6609F: // 6
      turnOn(1);
      colorVal = 400; // Cyan
      break;

      case 0x61D6E01F: // 7
      turnOn(1);
      colorVal = 630; // Light blue
      break;

      case 0x61D610EF: // 8
      turnOn(1);
      colorVal = 682; // Blue
      break;

      case 0x61D6906F: // 9
      turnOn(1);
      colorVal = 745; // Purple
      break;

      case 0x61D600FF: // 0
      turnOn(1);
      colorVal = 975; // Pink
      break;

      case 0x61D650AF: // Recall
      changeMode(1);
      break;

      case 0x61D628D7: // Menu
      break;

      case 0x61D6E817: // +100
      turnOn(1);
      powerVal = 510; // White
      break;

      case 0x61D608F7: // Info
      if (results.value != 0xFFFFFFFF) {
        Serial.println("--Info--");
        Serial.print("mode: ");
        Serial.println(lightMode, DEC);
        Serial.print("colorVal: ");
        Serial.println(colorVal, DEC);
        Serial.print("powerVal: ");
        Serial.println(powerVal, DEC);
        Serial.print("interval: ");
        Serial.println(interval, DEC);
        Serial.print("R: ");
        Serial.print(redPwr, DEC);
        Serial.print(" G: ");
        Serial.print(greenPwr, DEC);
        Serial.print(" B: ");
        Serial.println(bluePwr, DEC);
      }
      break;

      case 0x61D6D02F: // Up
      if (results.value != 0xFFFFFFFF) {
        interval++;
      } else {
        interval = interval+5;
      }
      if (interval > 4095) {
        interval = 4095;
      }
      break;

      case 0x61D6A857: // Down
      if (results.value != 0xFFFFFFFF) {
        interval--;
      } else {
        interval = interval-5;
      }
      if (interval < 0) {
        interval = 0;
      }
      break;

      case 0x61D618E7: // Left
      break;

      case 0x61D630CF: // Right
      break;

      case 0x61D66897: // OK
      interval = 50;
      break;

      case 0x61D68877: // TV/AV
      break;

      case 0x61D6B04F: // Mode
      if (results.value != 0xFFFFFFFF) {
        switch(lightMode) {
          case 0:
          changeMode(1);
          break;
          case 1:
          changeMode(2);
          break;
          case 2:
          changeMode(3);
          break;
          case 3:
          colorVal = 0;
          changeMode(4);
          break;          
          default:
          changeMode(1);
        }
      }
      break;

      case 0x61D6F00F: // Audio
      changeMode(2);
      break;

      case 0x61D6708F: // Sleep
      changeMode(10);
      break;

      case 0x61D6D827: // Vol+
      if (results.value != 0xFFFFFFFF) {
        powerVal++;
      } else {
        powerVal = powerVal+10;
      }
      if (powerVal > 510) {
        powerVal = 510;
      }
      break;

      case 0x61D6F807: // Vol-
      if (results.value != 0xFFFFFFFF) {
        powerVal--;
      } else {
        powerVal = powerVal-10;
      }
      if (powerVal < 0) {
        powerVal = 0;
      }
      break;

      case 0x61D6C837: // Mute
      powerVal = 255;
      break;

      case 0x61D658A7: // Chan+
      if (results.value != 0xFFFFFFFF) {
        colorVal++;
      } else {
        colorVal = colorVal+10;
      }
      if (colorVal > 1023) {
        colorVal = colorVal-1023;
      }
      break;

      case 0x61D67887: // Chan-
      if (results.value != 0xFFFFFFFF) {
        colorVal--;
      } else {
        colorVal = colorVal-10;
      }
      if (colorVal < 0) {
        colorVal = colorVal+1023;
      }
      break;

      default:
      break;
    }
    
    EEPROM.write(0, lightMode);
    
    saveBig(colorVal, 1, 2);
    
    saveBig(powerVal, 3, 4);
    
    saveBig(interval, 5, 6);
  } // End IR control

  if (lightMode == 0) {      // turn light off
    powerVal = 0;
    fadeTo(1);
  }
  if (lightMode == 1) {        // set fixed color
    /*setColor();
    setPower();

    writeLED();*/
    fadeTo(1);
  }

  if (lightMode == 2) {     // pulse fixed color
    if (millis() - previousMillis > interval) {
      // save the last time you blinked the LED 
      previousMillis = millis();
      setColor();
      
      if (pulseState > powerVal) {
        pulseState = powerVal;
      }
      
      if (pulseDir == 0) {
        pulseState--;
        
        if (pulseState == 0) {
          pulseDir = 1;
        }
      } else {
        pulseState++;
        
        if (pulseState >= powerVal) {
          pulseDir = 0;
        }
      }
      
      redPwr = map(redPwr, 0, 255, 0, pulseState);
      bluePwr = map(bluePwr, 0, 255, 0, pulseState);
      greenPwr = map(greenPwr, 0, 255, 0, pulseState);
      
      
      writeLED();
    }
  }

  if (lightMode == 3) {  // randomize colorNew and step colorPwr to it
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
      if ((redPwr == redNew) && (greenPwr == greenNew) && (bluePwr == blueNew)) {
            redNew = random(254);
            greenNew = random(254);
            blueNew = random(254);
      }
      
      writeLED();
    }
  }
  if (lightMode == 4) {  // Pride flag colors
    if (isColor(94,0,255)) {
      colorVal = 0;
    } else if (isColor(255,0,0)) {
      colorVal = 40;
    } else if (isColor(255,60,0)) {
      colorVal = 140;
    } else if (isColor(255,210,0)) {
      colorVal = 341;
    } else if (isColor(0,255,0)) {
      colorVal = 682;
    } else if (isColor(0,0,255)) {
      colorVal = 745;
    }
    //setColor();
    fadeTo(interval);
  }
  
  if (lightMode == 10) { // Sleep mode
    if (millis() - previousMillis > 8000) {
      previousMillis = millis();
      
      powerVal--;
      
      setColor();
      setPower();
      
      writeLED();
      
      if (powerVal == 0) {
        powerVal = 255;
        changeMode(0);
      }
    }
  }
}

void changeMode(int mode) {
  if ((lightMode == 0) && (mode != 0)) {
    powerVal = lastPowerVal;
  }
  
  switch(mode) {
    case 0:
    lastLightMode = lightMode;
    lastPowerVal = powerVal;
    EEPROM.write(7, lastLightMode);
    
    saveBig(lastPowerVal, 8, 9);
    break;
  }
  
  lightMode = mode;
}

void turnOn(int mode) {
  if (lightMode == 0) {
    changeMode(mode);
  }
}

void fadeTo(int fadeSpeed) {
  int colors[3];
  calcColor(colors);
  calcPower(colors);

  redNew = colors[0];
  greenNew = colors[1];
  blueNew = colors[2];

  if (millis() - previousMillis > fadeSpeed) {
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
    writeLED();
  }
}

void setColor() {
  int colors[3];
  calcColor(colors);

  redPwr = colors[0];
  greenPwr = colors[1];
  bluePwr = colors[2];
}
  
void setPower() {
  int colors[3] = {redPwr, greenPwr, bluePwr};
  calcPower(colors);
  
  redPwr = colors[0];
  greenPwr = colors[1];
  bluePwr = colors[2];
}

void writeLED() {
  // display the colors
  analogWrite(ledRed, redPwr);
  analogWrite(ledGreen, greenPwr);
  analogWrite(ledBlue, bluePwr);
}

boolean isColor(int red, int green, int blue) {
  if ((redPwr == red) && (greenPwr == green) && (bluePwr == blue)) {
    return true;
  } else {
    return false;
  }
}

void calcColor(int *colors) {
  int red;
  int green;
  int blue;
  
   // RED > ORANGE > YELLOW
  if (colorVal >= 0 && colorVal < 170) {
    red = 255;
    blue = 0;
    green = map(colorVal, 0, 170, 0, 255);
  }

    // YELLOW > LIME?? > GREEN 
  if (colorVal >= 170 && colorVal < 341) {
    green = 255;
    blue = 0;
    red = map(colorVal, 341, 170, 0, 255);
  }

    // GREEN > TURQOUISE
  if (colorVal >= 341 && colorVal < 511) {
    green = 255;
    red = 0;
    blue = map(colorVal, 341, 511, 0, 255);
  }

    // TURQOUISE > BLUE  
  if (colorVal >= 511 && colorVal < 682) {
    blue = 255;
    red = 0;
    green = map(colorVal, 682, 511, 0, 255);
  }

    // BLUE > PURPLE 
  if (colorVal >= 682 && colorVal < 852) {
    blue = 255;
    green = 0;
    red = map(colorVal, 682, 852, 0, 255);
  }

    // PURPLE > RED
  if (colorVal >= 852 && colorVal <= 1023) {
    red = 255;
    green = 0;
    blue = map(colorVal, 1023, 852, 0, 255);
  }

  colors[0] = red;
  colors[1] = green;
  colors[2] = blue;
}

void calcPower(int *colors) {
  int red = colors[0];
  int green = colors[1];
  int blue = colors[2];
  
  if (powerVal <= 255) {
    red = map(red, 0, 255, 0, powerVal);
    green = map(green, 0, 255, 0, powerVal);
    blue = map(blue, 0, 255, 0, powerVal);
  } else {
    int lowVal = powerVal-255;
    red = map(red, 0, 255, lowVal, 255);
    green = map(green, 0, 255, lowVal, 255);
    blue = map(blue, 0, 255, lowVal, 255);
  }
  
  colors[0] = red;
  colors[1] = green;
  colors[2] = blue;
}

void saveBig(int value, int address0, int address1) {
  int value1 = value / 255;
  int value2 = value % 255;
  
  EEPROM.write(address0, value1);
  EEPROM.write(address1, value2);
}

int loadBig(int address0, int address1) {
  int v1 = EEPROM.read(address0);
  int v2 = EEPROM.read(address1);
  return v1*255+v2; 
}
