// RC5 remote control receiver decoder.
// Tested with Philips or Philips compatible TV remotes.
// Developed by Alessandro Lambardi, 25/12/2007
// Released under Creative Commons license 2.5.
// Non-commercial use, attribution, share alike.
//
// Completely interrupt driven, no 'wait until' loops.
// When a valid code is received it is made available at
// variable data_word (main loop).
// External Arduino clock 16Mhz, hardware prescaler = 1
// Designed for AVR ATtiny24, adapted for Arduino.
//
// Program memory resources used (ATtiny24):
// 670 Program bytes circa out of 2048 (1/3 circa)
// 4 data bytes out of 128
//
// Internal hardware resources used:
// 8Bit Timer 0:	reset by software as required
// PORTA B, bit 0	can be relocated together with pin
//			change interrupt assignments

#define F_CPU 16000000UL	// CPU clock in Hertz.
#define TMR0_PRESCALER 256UL
#define IR_BIT 1778UL	// bit duration (us) in use for IR remote (RC5 std)
#define	IR_IN  8	//IR receiver is on digital pin 8 (PORT B, bit0)

#define TMR0_T (F_CPU/TMR0_PRESCALER*IR_BIT/1000000UL)
#define TMR0_Tmin (TMR0_T - TMR0_T/4UL)	// -25%
#define TMR0_Tmax (TMR0_T + TMR0_T/4UL)	// +25%
#if TMR0_Tmax > 255
	#error "TMR0_Tmax too big, change TMR0 prescaler value ", (TMR0_Tmax)
#endif

// Variables that are set inside interrupt routines and watched outside
// must be volatile
#include "WProgram.h"
void start_timer0(uint8_t cnt);
void setup();
void loop();
volatile uint8_t	tmr0_OC1A_int;  // flag, signals TMR0 timeout (bad !)
volatile uint8_t	no_bits;        // RC5 bits counter (0..14)
volatile uint16_t	ir_data_word;   // if <> holds a valid RC5 bits string (good !)

void start_timer0(uint8_t cnt) {
	OCR0A = cnt;
	TCNT0 = 0;
	tmr0_OC1A_int = 0;
	TIMSK0 |= _BV(OCIE0A);	// enable interrupt on OC0A match
	TCCR0B |= _BV(CS02);	// start timer0 with prescaler = 256
}

// Interrupt service routines
ISR(PCINT0_vect) {			//  signal handler for pin change interrupt
	if(no_bits == 0) {		// hunt for first start bit (must be == 1)
		if(!digitalRead(IR_IN)){
			start_timer0(TMR0_Tmax);
			no_bits++;
			ir_data_word = 1;
		}
	} else {
		if(!tmr0_OC1A_int) {		// not too much time,
			if(TCNT0 > TMR0_Tmin) {	// not too little.
				// if so wait next (mid bit) interrupt edge
				start_timer0(TMR0_Tmax);
				no_bits++;
				ir_data_word <<= 1;
				if(!digitalRead(IR_IN)){
					ir_data_word |= 1;
				} else {
					ir_data_word &= ~1;
				}
			}
		}
	}
}

ISR(TIM0_COMPA_vect) {		// timer0 OC1A match interrupt handler
	TCCR0B &= ~(_BV(CS02) | _BV(CS01) | _BV(CS00));	// stop timer
	tmr0_OC1A_int = 1;	// signal timeout
	no_bits = 0;		// start over with hunt for valid stream
	ir_data_word = 0;
}

const int ledPin =  13; 
int ledState = LOW;

void setup() {

        Serial.begin(9600);	// opens serial port, sets data rate to 9600 bps

// IR remote control receiver is on IR_IN digital port
        pinMode(IR_IN, INPUT);

        pinMode(4, OUTPUT);  // test output : LED for remote key "1"
        pinMode(5, OUTPUT);  // test output : LED for remote key "2"
        pinMode(6, OUTPUT);  // test output : LED for remote key "3"
        pinMode(7, OUTPUT);  // test output : LED for remote key "4"
        pinMode(ledPin, OUTPUT);
        digitalWrite(ledPin, ledState);

// pin change interrupt enable on IR_IN port
	PCMSK0 |= _BV(PCINT0);
	PCICR |= _BV(PCIE0);

// Timer 0 : Fast PWM mode. Stopped, for now.
	TCCR0A = _BV(WGM00) | _BV(WGM01);
	TCCR0B = _BV(WGM02);
	no_bits = 0;
	sei();			// enable interrupts
}

void loop() {
        
  if (ledState == HIGH) {
    ledState = LOW;
  } else {
    ledState = HIGH;
  }
  digitalWrite(ledPin, ledState);
  
	uint8_t data_word;    // 0..63 holds a valid RC5 key code.

	for(;;) {
		if((no_bits == 14) && ((ir_data_word & 0x37C0) == 0x3000)){
			no_bits = 0;						// prepare for next capture
			data_word = ir_data_word & 0x3F;	// extract data word

                        Serial.print(data_word, HEX);          // output code to console
                        Serial.print("\n\r");                    // output code to console
 
			switch(data_word) {
				case 0x01:				// button 1 on most RC5 remotes
				// do something
				digitalWrite(4,1);
				digitalWrite(5,1);
				digitalWrite(6,1);
				digitalWrite(7,0);
				break;
				case 0x02:				// button 2 on most RC5 remotes
				// do something
				digitalWrite(4,1);
				digitalWrite(5,1);
				digitalWrite(6,0);
				digitalWrite(7,1);
				break;
				case 0x03:				// button 3 on most RC5 remotes
				// do something
				digitalWrite(4,1);
				digitalWrite(5,0);
				digitalWrite(6,1);
				digitalWrite(7,1);
				break;
				case 0x04:				// button 4 on most RC5 remotes
				// do something
				digitalWrite(4,0);
				digitalWrite(5,1);
				digitalWrite(6,1);
				digitalWrite(7,1);
				break;
				case 0x0C:			// button POWER ON/OFF on most RC5 TV remotes
				// do something
				digitalWrite(4,1);
				digitalWrite(5,1);
				digitalWrite(6,1);
				digitalWrite(7,1);
				break;
			// etcetera for other codes,  some are in the table below
			}
		}
	} 
}

/* Typical RC5 codes:
key			command bits (system bits = 0)
0			0x00
1			0x01
2			0x02
3			0x03
4			0x04
5			0x05
6			0x06
7			0x07
8			0x08
9			0x09
"right arrow"		0x10
"left arrow"		0x11
"channel shuffle +"	0x20
"channel shuffle +"	0x21
"mute"			0x0D
sleep timer		0x26
vol -			0x11
vol +			0x10
"auto fine tune"	0x3B
default restore		0x0E
power			0x0C
*/

int main(void)
{
	init();

	setup();
    
	for (;;)
		loop();
        
	return 0;
}

