#include <IRremote.h>
#include <LED.h>

int RECV_PIN = 7;

const byte NUMBER_OF_LEDS = 4;

LED led[NUMBER_OF_LEDS] = { LED(2), LED(3), LED(4), LED(5) };

int currentLED = 0;
int volumeLED = 0;
int chanLED = 0;

int mode = 0;

int lastCode;

IRrecv irrecv(RECV_PIN);

decode_results results;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  chanMode();
  switchLED(chanLED);
}

void loop() {
  if (irrecv.decode(&results)) {
    //erial.println(results.value, HEX);
    irrecv.resume(); // Receive the next value
    
    if (results.value != 0xFFFFFFFF) {
      lastCode = results.value;
    }
    
    switch(lastCode) {
      case 0x61D638C7: // FM
      break;
      case 0x61D648B7: // Power
        powerOff();
      break;
      case 0x61D6807F: // 1
        chanMode();
        chanLED = 0;
        switchLED(chanLED);
      break;
      case 0x61D640BF: // 2
        chanMode();
        chanLED = 1;
        switchLED(chanLED);
      break;
      case 0x61D6C03F: // 3
        chanMode();
        chanLED = 2;
        switchLED(chanLED);
      break;
      case 0x61D620DF: // 4
        chanMode();
        chanLED = 3;
        switchLED(chanLED);
      break;
      case 0x61D6A05F: // 5
      break;
      case 0x61D6609F: // 6
      break;
      case 0x61D6E01F: // 7
      break;
      case 0x61D610EF: // 8
      break;
      case 0x61D6906F: // 9
      break;
      case 0x61D600FF: // 0
      break;
      case 0x61D650AF: // Recall
      break;
      case 0x61D628D7: // Menu
      break;
      case 0x61D6E817: // +100
      break;
      case 0x61D608F7: // Info
      break;
      case 0x61D6D02F: // Up
      break;
      case 0x61D6A857: // Down
      break;
      case 0x61D618E7: // Left
      break;
      case 0x61D630CF: // Right
      break;
      case 0x61D66897: // OK
      break;
      case 0x61D68877: // TV/AV
      break;
      case 0x61D6B04F: // Mode
        toggleMode();
      break;
      case 0x61D6F00F: // Audio
      break;
      case 0x61D6708F: // Sleep
      break;
      case 0x61D6D827: // Vol+
        volMode();
        led[volumeLED].on();
        if (volumeLED < NUMBER_OF_LEDS-1) {
          volumeLED++;
        }
      break;
      case 0x61D6F807: // Vol-
        volMode();
        led[volumeLED].off();
        if (volumeLED > 0) {
          volumeLED--;
        }
      break;
      case 0x61D6C837: // Mute
      break;
      case 0x61D658A7: // Chan+
        chanMode();
        if (chanLED < NUMBER_OF_LEDS-1) {
          chanLED++;
        } else {
          chanLED = 0;
        }
        switchLED(chanLED);
      break;
      case 0x61D67887: // Chan-
        chanMode();
        if (chanLED > 0) {
          chanLED--;
        } else {
          chanLED = 3;
        }
        switchLED(chanLED);
      break;
      
      case 0xFFFFFFFF: // repeat
      break;
      default:
        Serial.print("Unknown code: ");
        Serial.println(results.value, HEX);
      break;
    }
  }
}

void switchLED(int thisLed) {
  allOff();
  led[thisLed].on();
}
void allOff() {
  for (currentLED = 0; currentLED < NUMBER_OF_LEDS; currentLED++) {
    led[currentLED].off();
  }
}
void powerOff() {
 allOff();
 volumeLED = 0;
 chanLED = -1; 
}
void volMode() {
  allOff();
  for (currentLED = 0; currentLED < volumeLED; currentLED++) {
    led[currentLED].on();
  }
}
void chanMode() {
 allOff();
 switchLED(chanLED);
}

void toggleMode() {
  switch (mode){
    case 0: // Currently chanMode
      volMode();
      mode = 1;
    break;
    case 1: // Currently volMode
      chanMode();
      mode = 0;
    break;
  }
}
