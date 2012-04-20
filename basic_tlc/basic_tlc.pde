#include "Tlc5940.h"

void setup()
{
  Serial.begin(9600);
  Tlc.init(4095);
  Tlc.update();
}

void loop()
{
  setLamp1(0, 4095, 1024);
  setLamp2(4095, 1024, 0);
  Tlc.update();
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

void setLamp1(int red, int green, int blue) {
  setPower(0, red);
  setPower(1, green);
  setPower(2, blue);
}

void setLamp2(int red, int green, int blue) {
  setPower(3, red);
  setPower(4, green);
  setPower(5, blue);
}

void setLamp3(int red, int green, int blue) {
  setPower(6, red);
  setPower(7, green);
  setPower(8, blue);
}

void setLamp4(int red, int green, int blue) {
  setPower(9, red);
  setPower(10, green);
  setPower(11, blue);
}

void setRed(int power) {
  setPower(0, power);
  setPower(3, power);
  setPower(6, power);
  setPower(9, power);
}

void setGreen(int power) {
  setPower(1, power);
  setPower(4, power);
  setPower(7, power);
  setPower(10, power);
}

void setBlue(int power) {
  setPower(2, power);
  setPower(5, power);
  setPower(8, power);
  setPower(11, power);
}

void setPower(int channel, int power) {
  power = map(power, 4095, 0, 0, 4095);
  Tlc.set(channel, power);
}
