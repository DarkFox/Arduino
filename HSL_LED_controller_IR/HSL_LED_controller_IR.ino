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

byte serialMode = 0;
byte getSerialBigInt = 0;

int serialBigInt1;
int serialBigInt2;

// LED Power variables
byte redPwr = 0;
byte greenPwr = 0;
byte bluePwr = 0;

// Variables for colors to fade to
// set the initial random colors
byte redNew = random(255);
byte greenNew = random(255);
byte blueNew = random(255);

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

int hueVal;
int satVal;
int lumVal;

// misc interface variables
byte menu = 0;

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
  
  hueVal = loadBig(1,2);
  satVal = EEPROM.read(3);
  lumVal = EEPROM.read(4);

  interval = loadBig(5,6);

  lastLightMode = EEPROM.read(7);
}

void loop()
{
  getIrCmd();
  getSerialCmd();

  switch(lightMode) {
    case 0:
    mdOff();
    break;
    case 1:
    mdSolid();
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
    case 6:
    mdSleep();
    break;
    case 99:
    // Freewheeling, let serial run the show. 
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
void mdSolid() {
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
      
      if (pulseState >= 127) {
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
      lumVal = 127;
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
      lumVal = 127;
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
        hueVal = random(360);
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
      if (menu == 1) {
        changeMode(1);
      } else {
        turnOn(1);
        hueVal = 0; // Red
      }
      break;
 
      case 0x61D640BF: // 2
      if (menu == 1) {
        changeMode(2);
      } else {
        turnOn(1);
        hueVal = 120; // Green
      }
      break;

      case 0x61D6C03F: // 3
      if (menu == 1) {
        changeMode(3);
      } else {
        turnOn(1);
        hueVal = 240; // Blue
      }
      break;

      case 0x61D620DF: // 4
      if (menu == 1) {
        changeMode(4);
      } else {
        turnOn(1);
        hueVal = 10; // Orange
      }
      break;

      case 0x61D6A05F: // 5
      if (menu == 1) {
        changeMode(5);
      } else {
        turnOn(1);
        hueVal = 135; // Sea Foam
      }
      break;

      case 0x61D6609F: // 6
      if (menu == 1) {
        changeMode(6);
      } else {
        turnOn(1);
        hueVal = 270; // Purple
      }
      break;

      case 0x61D6E01F: // 7
      turnOn(1);
      hueVal = 45; // Yellow
      break;

      case 0x61D610EF: // 8
      turnOn(1);
      hueVal = 180; // Cyan
      break;

      case 0x61D6906F: // 9
      turnOn(1);
      hueVal = 350; // Pink
      break;

      case 0x61D600FF: // 0
      turnOn(1);
      hueVal = 210; // Light blue
      break;

      case 0x61D650AF: // Recall
      turnOn(1);
      hueVal = 80; // Lime
      break;

      case 0x61D628D7: // Menu
      if (menu == 0) {
        menu = 1;
      } else {
        menu = 0;
      }
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
        satVal++;
      } else {
        satVal = satVal+10;
      }
      if (satVal > 255) {
        satVal = 255;
      }
      break;

      case 0x61D6A857: // Down
      if (results.value != 0xFFFFFFFF) {
        satVal--;
      } else {
        satVal = satVal-10;
      }
      if (satVal < 0) {
        satVal = 0;
      }
      break;

      case 0x61D618E7: // Left
      if (results.value != 0xFFFFFFFF) {
        hueVal--;
      } else {
        hueVal = hueVal-10;
      }
      if (hueVal < 0) {
        hueVal = hueVal+360;
      }
      break;

      case 0x61D630CF: // Right
      if (results.value != 0xFFFFFFFF) {
        hueVal++;
      } else {
        hueVal = hueVal+10;
      }
      if (hueVal > 360) {
        hueVal = hueVal-360;
      }
      break;

      case 0x61D66897: // OK
      satVal = 255;
      break;

      case 0x61D68877: // TV/AV
      break;

      case 0x61D6B04F: // Mode
      interval = 50;
      break;

      case 0x61D6F00F: // Audio
      break;

      case 0x61D6708F: // Sleep
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
      lumVal = 127;
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

void getSerialCmd() {
  // check if data has been sent from the computer:
  if (Serial.available() > 0) {
    switch (serialMode) {
      case 0:
        // Set serial mode.
        serialMode = Serial.read();
        Serial.write(1);
        break;
      case 1:
        // Change light mode.
        changeMode(Serial.read());
        serialMode = 0;
        Serial.write(1);
        break;
      case 2:
        // Set hue, 2 (2 rounds)
        if (getSerialBigInt == 0) {
          serialBigInt1 = Serial.read();
          getSerialBigInt = 1;
        } else {
          serialBigInt2 = Serial.read();
          hueVal = serialBigInt1 * 255 + serialBigInt2;
          getSerialBigInt = 0;
          serialMode = 0;
        }
        Serial.write(1);
        break;
      case 3:
        // Set saturation
        satVal = Serial.read();
        serialMode = 0;
        Serial.write(1);
        break;
      case 4:
        // Set luminescence
        lumVal = Serial.read();
        serialMode = 0;
        Serial.write(1);
        break;
      case 5:
        // Set interval
        if (getSerialBigInt == 0) {
          serialBigInt1 = Serial.read();
          getSerialBigInt = 1;
        } else {
          serialBigInt2 = Serial.read();
          interval = serialBigInt1 * 255 + serialBigInt2;
          getSerialBigInt = 0;
          serialMode = 0;
        }
        Serial.write(1);
        break;
    }
  }
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
  switch(mode) {
    case 0:
    lastLightMode = lightMode;
    EEPROM.write(7, lastLightMode);
    case 1:
    lumVal = 127;
    case 3:
    lumVal = 127;
    case 4:
    hueVal = 0;
    case 5:
    lumVal = 127;
    break;
  }
  
  lightMode = mode;
  menu = 0;
}

void turnOn(int mode) {
  if (lightMode == 0) {
    changeMode(mode);
  }
}

void fadeTo(int fadeSpeed) {
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
void hslToRgb( int hue, byte sat, byte lum, byte* lpOutRgb )
{
  if( sat == 0 )
  {
    lpOutRgb[0] = lpOutRgb[1] = lpOutRgb[2] = lum;
    return;
  }

  float nhue   = (float)hue * (1.0f / 360.0f);
  float nsat   = (float)sat * (1.0f / 255.0f);
  float nlum   = (float)lum * (1.0f / 255.0f);
  
  float m2;
  if( lum < 128 ) 
    m2 = nlum * ( 1.0f + nsat );
  else           
    m2 = ( nlum + nsat ) - ( nsat * nlum );
 
  float m1 = ( 2.0f * nlum ) - m2;
 
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
