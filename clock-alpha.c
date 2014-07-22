#define F_CPU 4e6

#include <avr/io.h>
//#include <stdio.h>
#include <avr/sfr_defs.h>
#include <avr/iotn2313.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// change these
static unsigned char hours = 3;
static unsigned char minutes = 13;
static unsigned char seconds = 0;

// don't change these. srsly.
static unsigned char ticks = 0;
static int buttonticksS = -1;
static int buttonticksM = -1;

// button debounce timer, every 4ms
ISR(TIMER0_OVF_vect)
{
	if (PIND & (1<<0)) {
		buttonticksS++;
	} else {
		if (buttonticksS != -1) {
			if (buttonticksS > 5) {
				minutes = 0;
				hours++;
			}
			buttonticksS = -1;
		}
	}

	if (PIND & (1<<1)) {
		buttonticksM++;
	} else {
		if (buttonticksM != -1) {
			if (buttonticksM > 5) {
				seconds = 0;
				minutes++;
			}
			buttonticksM = -1;
		}
	}
}

ISR(TIMER1_COMPA_vect)
{
	ticks++;

	if (ticks >= 100)
		ticks = 0, seconds++;

	if (seconds >= 60)
		seconds -= 60, minutes++;

	if (minutes >= 60)
		minutes -= 60, hours++;

	if (hours >= 24)
		hours -= 24;
}

void displayPart(unsigned char d, unsigned char value)
{
	PORTB = ((value & (1<<0)) << 4)
	      | ((value & (1<<1)) << 2)
	      | ((value & (1<<2)) << 0)
	      | ((value & (1<<3)) >> 2)
	      | ((value & (1<<4)) >> 4);
	PORTD = d | ((value & (1<<5)) << 1);

	_delay_us(5000);
}

int main(void)
{
	DDRA = 0b00000000;
	DDRB = 0b00011111;
	DDRD = 0b01111000;

	//PORTA = 0b100;
	//PORTD = 0b11; // OMFG!

	// timer0 for button polling (every 4ms)
	TCCR0A = 0;
	TCCR0B = (1<<WGM02) | (1<<CS01) | (1<<CS00); // ctc mode, 64 prescaler

	// timer1 for time ticks
	TCCR1A = 0;
	TCCR1B = (1<<WGM12) | (1<<CS10); // ctc mode, no prescaler
	OCR1A = 39999; // set interval

	// general timer setup
	TIMSK = (1<<TOIE0) | (1<<OCIE1A); // enable interrupts
	TIFR = (1<<TOV0) | (1<<OCF1A); // clear pending interrupts
	//TIMSK = (1<<OCIE1A); // enable interrupts
	//TIFR = (1<<OCF1A); // clear pending interrupts

	sei();

	// multiplex forever
	while (1) {
		displayPart(0b00110011, hours);
		displayPart(0b00101011, minutes);
		displayPart(0b00011011, seconds);
	}
}
