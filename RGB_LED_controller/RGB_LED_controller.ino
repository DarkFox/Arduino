#include <stdarg.h>
#include <EEPROM.h>
#include <IRremote.h>
#include <IRremoteInt.h>

/* 
RGB LED IR controller for Arduino
By Martin "DarkFox" Eberhardt 2009-12-29
http://darkfox.dk/

Based on RGB LED controller by Markus Ulfberg 2009-05-19
http://genericnerd.blogspot.com/2009/05/arduino-mood-light-controller.html
*/

// set the ledPins
int redPin = 5;
int greenPin = 6;
int bluePin = 9;

int lastCode;
int code;
int RECV_PIN = 2;
IRrecv irrecv(RECV_PIN);
decode_results results;

// LED Power variables
byte redPwr = 0;
byte greenPwr = 0;
byte bluePwr = 0;

// light mode variable
// initial value 0 = off
int lightMode = 0; // Saved in EEPROM address 0

// Variables for saving the last state before standby
int lastLightMode = 1; // EEPROM address 7

// Variables for colors to fade to
// set the initial random colors
int hueNew = random(360);
int satNew = random(255);
int lumNew = random(255);


// misc interface variables
int hueVal;
int satVal;
int lumVal;

long previousDelayMillis = 0; // Will store last time button was pressed
long buttonDelay = 200;        // Delay before repeating button press

long previousMillis = 0;      // will store last time thing was updated
long interval = 10;           // interval at which to blink (milliseconds) (0 - 4095) Saved in EEPROM address 5+6

int pulseState = 255; // Save where the pulse is. Starting high.
int pulseDir = 0;     // Pulse direction

void setup()
{  
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
 
  loadConfig();

  //pinMode(potPin, INPUT);

  irrecv.enableIRIn(); // Start the receiver
  
  lightMode = 1;
  
  // serial
  Serial.begin(9600);
  Serial.println("--RGB LED controller booted and ready--");
}

void loadConfig() {
  lightMode = EEPROM.read(0);
  
  interval = loadBig(5,6);
  
  lastLightMode = EEPROM.read(7);

  hueVal = loadBig(1,2);
  satVal = EEPROM.read(3);
  lumVal = EEPROM.read(4);
}

void loop()
{
  getIrCmd();

  switch(lightMode) {
    case 0:
    mdOff();
    break;
    case 1:
    mdOn();
    break;
    case 2:
    mdPulse();
    break;
    case 3:
    mdRandomFade();
    break;
    case 4:
    mdRainbow();
    break;
    case 5:
    mdStrobe();
    break;
    case 10:
    mdSleep();
    break;    
    default:
    mdOff();
  }
}

// Off
void mdOff() {
  lumVal = 0;
  fadeTo(1);
}

// On - Solid
void mdOn() {
  lumVal = 127;
  fadeTo(1);
}

// Pulsing
void mdPulse() {
  if (millis() - previousMillis > interval) {
    previousMillis = millis();
    
    if (pulseState > lumVal) {
      pulseState = lumVal;
    }
    
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
    
    lumVal = pulseState;

    hslWrite();
  }
}

// Fade to random
void mdRandomFade() {
  if (millis() - previousMillis > interval) {
    previousMillis = millis();
    
    if (hueVal > hueNew) {
      hueVal--;
    } 
    if (hueVal < hueNew) {
      hueVal++;
    }
    if (satVal > satNew) {
      satVal--;
    } 
    if (satVal < satNew) {
      satVal++;
    }

    // If all Pwr match New get new colors
    if ((hueVal == hueNew) && (satVal == satNew)) {
      hueNew = random(254);
      satNew = random(254);
    }
    
    hslWrite();
  }
}

// Rainbow flag fade
void mdRainbow() {
  switch(hueVal) {
    case 0:
    hueVal = 30;
    case 30:
    hueVal = 60;
    case 60:
    hueVal = 120;
    case 120:
    hueVal = 240;
    case 240:
    hueVal = 300;
    case 300:
    hueVal = 0;
  }

  fadeTo(interval);
}

// Strobe
void mdStrobe() {
  if (millis() - previousMillis > interval) {
    previousMillis = millis();
    
    if (lumVal == 0) {
      lumVal = 255;
    } else {
      lumVal = 0;
    }

    hslWrite();
  }
}

// Sleep
void mdSleep() {
  if (millis() - previousMillis > 8000) {
    previousMillis = millis();
    
    lumVal--;
    
    hslWrite();
    
    if (lumVal == 0) {
      lumVal = 255;
      changeMode(0);
    }
  }
}

void getIrCmd() {
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
        hueVal = random(1023);
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
      hueVal = 0; // Red
      break;
 
      case 0x61D640BF: // 2
      turnOn(1);
      hueVal = 120; // Green
      break;

      case 0x61D6C03F: // 3
      turnOn(1);
      hueVal = 240; // Blue
      break;

      case 0x61D620DF: // 4
      turnOn(1);
      hueVal = 30; // Orange
      break;

      case 0x61D6A05F: // 5
      turnOn(1);
      hueVal = 150; // Sea Foam
      break;

      case 0x61D6609F: // 6
      turnOn(1);
      hueVal = 300; // Purple
      break;

      case 0x61D6E01F: // 7
      turnOn(1);
      hueVal = 60; // Yellow
      break;

      case 0x61D610EF: // 8
      turnOn(1);
      hueVal = 180; // Cyan
      break;

      case 0x61D6906F: // 9
      turnOn(1);
      hueVal = 330; // Pink
      break;

      case 0x61D600FF: // 0
      turnOn(1);
      hueVal = 210; // Light blue
      break;

      case 0x61D650AF: // Recall
      turnOn(1);
      hueVal = 90; // Lime
      break;

      case 0x61D628D7: // Menu
      changeMode(1);
      break;

      case 0x61D6E817: // +100
      turnOn(1);
      satVal = 0; // White
      break;

      case 0x61D608F7: // Info
      if (results.value != 0xFFFFFFFF) {
        printInfoToSerial();
      }
      break;

      case 0x61D6D02F: // Up
      if (results.value != 0xFFFFFFFF) {
        hueVal++;
      } else {
        hueVal = hueVal+10;
      }
      if (hueVal > 360) {
        hueVal = hueVal-360;
      }
      break;

      case 0x61D6A857: // Down
      if (results.value != 0xFFFFFFFF) {
        hueVal--;
      } else {
        hueVal = hueVal-10;
      }
      if (hueVal < 0) {
        hueVal = hueVal+360;
      }
      break;

      case 0x61D618E7: // Left
      if (results.value != 0xFFFFFFFF) {
        satVal++;
      } else {
        satVal = satVal+10;
      }
      if (satVal > 255) {
        satVal = 255;
      }
      break;

      case 0x61D630CF: // Right
      if (results.value != 0xFFFFFFFF) {
        satVal--;
      } else {
        satVal = satVal-10;
      }
      if (satVal < 0) {
        satVal = 0;
      }
      break;

      case 0x61D66897: // OK
      satVal = 127;
      break;

      case 0x61D68877: // TV/AV
      interval = 50;
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
          lumVal = 255;
          changeMode(5);
          break;
          case 5:
          lumVal = 255;
          changeMode(3);
          break;
          case 3:
          hueVal = 0;
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
        lumVal++;
      } else {
        lumVal = lumVal+10;
      }
      if (lumVal > 255) {
        lumVal = 255;
      }
      break;

      case 0x61D6F807: // Vol-
      if (results.value != 0xFFFFFFFF) {
        lumVal--;
      } else {
        lumVal = lumVal-10;
      }
      if (lumVal < 0) {
        lumVal = 0;
      }
      break;

      case 0x61D6C837: // Mute
      lumVal = 255;
      break;

      case 0x61D658A7: // Chan+
      if (results.value != 0xFFFFFFFF) {
        interval++;
      } else {
        interval = interval+5;
      }
      if (interval > 4095) {
        interval = 4095;
      }
      break;

      case 0x61D67887: // Chan-
      if (results.value != 0xFFFFFFFF) {
        interval--;
      } else {
        interval = interval-5;
      }
      if (interval < 0) {
        interval = 0;
      }
      break;

      default:
      break;
    }
    
    saveState();
  } // End IR control
}

void saveState() {
  EEPROM.write(0, lightMode);
  saveBig(hueVal, 1, 2);
  EEPROM.write(3, satVal);
  EEPROM.write(4, lumVal);
  saveBig(interval, 5, 6);
}

void printInfoToSerial() {
  Serial.println("--Info--");
  Serial.print("mode: ");
  Serial.println(lightMode, DEC);
  Serial.print("hueVal: ");
  Serial.println(hueVal, DEC);
  Serial.print("satVal: ");
  Serial.println(satVal, DEC);
  Serial.print("lumVal: ");
  Serial.println(lumVal, DEC);
  Serial.print("interval: ");
  Serial.println(interval, DEC);
}

void changeMode(int mode) {
  if ((lightMode == 0) && (mode != 0)) {
    lumVal = lastlumVal;
  }
  
  switch(mode) {
    case 0:
    lastLightMode = lightMode;
    lastlumVal = lumVal;
    EEPROM.write(7, lastLightMode);
    
    saveBig(lastlumVal, 8, 9);
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
  int rgb[3];

  byte rgb[3];
  hslToRgb( hueVal, satVal, lumVal, rgb );

  redNew = rgb[0];
  greenNew = rgb[1];
  blueNew = rgb[2];

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


//-------------------------------------------------------------------
// -- hsl output --
//

void hslWrite()
{
    byte rgb[3];
    hslToRgb( hueVal, satVal, lumVal, rgb );
    
    redPwr = rgb[0];
    greenPwr = rgb[1];
    bluePwr =  rgb[2];
    writeLED();
}

// from http://www.kasperkamperman.com/blog/arduino/arduino-programming-hsb-to-rgb/
const byte gamma_curve[] = {
    0,   1,   1,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,
    3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   4,   4,   4,   4,
    4,   4,   4,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   6,   6,   6,
    6,   6,   6,   6,   6,   7,   7,   7,   7,   7,   7,   7,   8,   8,   8,   8,
    8,   8,   9,   9,   9,   9,   9,   9,   10,  10,  10,  10,  10,  11,  11,  11,
    11,  11,  12,  12,  12,  12,  12,  13,  13,  13,  13,  14,  14,  14,  14,  15,
    15,  15,  16,  16,  16,  16,  17,  17,  17,  18,  18,  18,  19,  19,  19,  20,
    20,  20,  21,  21,  22,  22,  22,  23,  23,  24,  24,  25,  25,  25,  26,  26,
    27,  27,  28,  28,  29,  29,  30,  30,  31,  32,  32,  33,  33,  34,  35,  35,
    36,  36,  37,  38,  38,  39,  40,  40,  41,  42,  43,  43,  44,  45,  46,  47,
    48,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,
    63,  64,  65,  66,  68,  69,  70,  71,  73,  74,  75,  76,  78,  79,  81,  82,
    83,  85,  86,  88,  90,  91,  93,  94,  96,  98,  99,  101, 103, 105, 107, 109,
    110, 112, 114, 116, 118, 121, 123, 125, 127, 129, 132, 134, 136, 139, 141, 144,
    146, 149, 151, 154, 157, 159, 162, 165, 168, 171, 174, 177, 180, 183, 186, 190,
    193, 196, 200, 203, 207, 211, 214, 218, 222, 226, 230, 234, 238, 242, 248, 255,
};


// http://www.dipzo.com/wordpress/?p=50
void hslToRgb( int hue, byte sat, byte light, byte* lpOutRgb )
{
  if( sat == 0 )
  {
    lpOutRgb[0] = lpOutRgb[1] = lpOutRgb[2] = light;
    return;
  }

  float nhue   = (float)hue * (1.0f / 360.0f);
  float nsat   = (float)sat * (1.0f / 255.0f);
  float nlight = (float)light * (1.0f / 255.0f);
  
  float m2;
  if( light < 128 ) 
    m2 = nlight * ( 1.0f + nsat );
  else           
    m2 = ( nlight + nsat ) - ( nsat * nlight );
 
  float m1 = ( 2.0f * nlight ) - m2;
 
  lpOutRgb[0] = hueToChannel( m1, m2, nhue + (1.0f / 3.0f) );
  lpOutRgb[1] = hueToChannel( m1, m2, nhue );
  lpOutRgb[2] = hueToChannel( m1, m2, nhue - (1.0f / 3.0f) );
}

inline byte hueToChannel( float m1, float m2, float h )
{
  return hueToChannelRaw( m1, m2, h );
}

byte hueToChannelRaw( float m1, float m2, float h )
{
  float channel = hueToChannelInternal( m1, m2, h );
  byte  uchan   = (byte)(255.0f * channel);
  return uchan;
}

// this gives much better colour matching than Raw, but results in more jumpy fading
byte hueToChannelGamma( float m1, float m2, float h )
{
  float channel = hueToChannelInternal( m1, m2, h );
  byte  uchan   = (byte)(255.0f * channel);
  return gamma_curve[uchan];
}

float hueToChannelInternal( float m1, float m2, float h )
{
   if( h < 0.0f ) h += 1.0f;
   if( h > 1.0f ) h -= 1.0f;
   
   if( ( 6.0f * h ) < 1.0f )  return ( m1 + ( m2 - m1 ) * 6.0f * h );
   if( ( 2.0f * h ) < 1.0f )  return m2;
   if( ( 3.0f * h ) < 2.0f )  return ( m1 + ( m2 - m1 ) * ((2.0f/3.0f) - h) * 6.0f );
   return m1;
}

void writeLED() {
  // display the colors
  analogWrite(redPin, redPwr);
  analogWrite(greenPin, greenPwr);
  analogWrite(bluePin, bluePwr);
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
  if (hueVal >= 0 && hueVal < 170) {
    red = 255;
    blue = 0;
    green = map(hueVal, 0, 170, 0, 255);
  }

    // YELLOW > LIME?? > GREEN 
  if (hueVal >= 170 && hueVal < 341) {
    green = 255;
    blue = 0;
    red = map(hueVal, 341, 170, 0, 255);
  }

    // GREEN > TURQOUISE
  if (hueVal >= 341 && hueVal < 511) {
    green = 255;
    red = 0;
    blue = map(hueVal, 341, 511, 0, 255);
  }

    // TURQOUISE > BLUE  
  if (hueVal >= 511 && hueVal < 682) {
    blue = 255;
    red = 0;
    green = map(hueVal, 682, 511, 0, 255);
  }

    // BLUE > PURPLE 
  if (hueVal >= 682 && hueVal < 852) {
    blue = 255;
    green = 0;
    red = map(hueVal, 682, 852, 0, 255);
  }

    // PURPLE > RED
  if (hueVal >= 852 && hueVal <= 1023) {
    red = 255;
    green = 0;
    blue = map(hueVal, 1023, 852, 0, 255);
  }

  colors[0] = red;
  colors[1] = green;
  colors[2] = blue;
}

void calcPower(int *colors) {
  int red = colors[0];
  int green = colors[1];
  int blue = colors[2];
  
  if (lumVal <= 255) {
    red = map(red, 0, 255, 0, lumVal);
    green = map(green, 0, 255, 0, lumVal);
    blue = map(blue, 0, 255, 0, lumVal);
  } else {
    int lowVal = lumVal-255;
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
