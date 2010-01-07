#include <LED.h>
#include <Button.h>
#include <Potentiometer.h>
/* Analog In and Out
 * ------------
 * by Jeff Gray
 *
 * Turns on two LEDs, one based digitally (on or off) and one based on a potentiometer
 * or analog device of some sort, giving a value from 0 (GND) to 5V.
 */

//Pins           
Button button = Button(2,PULLUP);
Potentiometer potentiometer = Potentiometer(0);
LED pwmled = LED(9);
LED led = LED(11);
int ledState = 0;

void setup()
{
  Serial.begin(9800);
  Serial.println("Potentiometer example");
  delay(2000);
}

void loop()
{
  pwmled.setValue(potentiometer.getValue()/4);
  
  if(button.uniquePress()){ //button is pressed, need to release it for this to evaluate true again
    if(ledState == 1){
      led.fadeOut(500);
      ledState = 0;
    } else {
      led.fadeIn(500);
      ledState = 1;
    }
  }
  
}
