#include "Tlc5940.h"

int colorWheelVal[4];
int powerVal[12];
int delayVal = 100;

void setup()
{
  Serial.begin(9600);
  Tlc.init(4095);
}

void loop()
{
  setLamp(0, 0, 4095, 0);
  setLamp(1, 4095, 4095, 0);
  setLamp(2, 4095, 0, 0);
  setLamp(3, 0, 0, 4095);
  updateTlc();
}

void colorWheel() {
  for (int i = 0; i < 4; i++) {
    colorWheelVal[i]++;
    if (colorWheelVal[i] > 1023) {
      colorWheelVal[i] = 0;
    }
    int colors[3];
    calcColorWheel(colors, colorWheelVal[i]);
    setLamp(i, colors[0], colors[1], colors[2]);
  }
  updateTlc();
  delay(delayVal);
}

void calcColorWheel(int *colors, int colorVal) {
  int red;
  int green;
  int blue;
  
   // RED > ORANGE > YELLOW
  if (colorVal >= 0 && colorVal < 170) {
    red = 4095;
    blue = 0;
    green = map(colorVal, 0, 170, 0, 4095);
  }
 
    // YELLOW > LIME?? > GREEN 
  if (colorVal >= 170 && colorVal < 341) {
    green = 4095;
    blue = 0;
    red = map(colorVal, 341, 170, 0, 4095);
  }
 
    // GREEN > TURQOUISE
  if (colorVal >= 341 && colorVal < 511) {
    green = 4095;
    red = 0;
    blue = map(colorVal, 341, 511, 0, 4095);
  }
 
    // TURQOUISE > BLUE  
  if (colorVal >= 511 && colorVal < 682) {
    blue = 4095;
    red = 0;
    green = map(colorVal, 682, 511, 0, 4095);
  }
 
    // BLUE > PURPLE 
  if (colorVal >= 682 && colorVal < 852) {
    blue = 4095;
    green = 0;
    red = map(colorVal, 682, 852, 0, 4095);
  }
 
    // PURPLE > RED
  if (colorVal >= 852 && colorVal <= 1023) {
    red = 4095;
    green = 0;
    blue = map(colorVal, 1023, 852, 0, 4095);
  }
 
  colors[0] = red;
  colors[1] = green;
  colors[2] = blue;
}

void setAll(int red, int green, int blue) {
  setPower(0, red);
  setPower(1, green);
  setPower(2, blue);
  setPower(3, red);
  setPower(4, green);
  setPower(5, blue);
  setPower(6, red);
  setPower(7, green);
  setPower(8, blue);
}

void setLamp(int lamp, int red, int green, int blue) {
  switch (lamp) {
    case 0:
      setPower(0, red);
      setPower(1, green);
      setPower(2, blue);
      break;
    case 1:
      setPower(3, red);
      setPower(4, green);
      setPower(5, blue);
      break;
    case 2:
      setPower(6, red);
      setPower(7, green);
      setPower(8, blue);
      break;
    case 3:
      setPower(9, red);
      setPower(10, green);
      setPower(11, blue);
      break;
  }
}

void setPower(int channel, int power) {
  powerVal[channel] = map(power, 4095, 0, 0, 4095);
}

void updateTlc() {
  for(int i = 0; i < 12; i++) {
    Tlc.set(i, powerVal[i]);
  }
  Tlc.update();
}
