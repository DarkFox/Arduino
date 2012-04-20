#include <stdarg.h>
#include "EEPROM.h"

void setup()
{
  

  // serial
  Serial.begin(9600);
  Serial.println("--EEPROM cleared--");
}


void loop()
{
  EEPROM.write(0, 0);
  EEPROM.write(1, 0);
  EEPROM.write(2, 0);
  EEPROM.write(3, 0);
  EEPROM.write(4, 0);
  EEPROM.write(5, 0);
  EEPROM.write(6, 0);
  EEPROM.write(7, 0);
  EEPROM.write(8, 0);
  EEPROM.write(9, 0);
  EEPROM.write(10, 0);
  EEPROM.write(11, 0);
  EEPROM.write(12, 0);
}
