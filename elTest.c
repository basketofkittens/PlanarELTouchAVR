#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>

#include "planarTouch.h"

// Init touchscreen
void touchInit(void) {

	EICRA &= ~( (1 << ISC01) | ~(1 << ISC00));	// Trig on INT0 low level
	//EICRA |= (1 << ISC01);	// Trig on INT0 falling edge
	//EICRA |= ((1 << ISC01) | (1 << ISC00));	// Trig on INT0 rising edge edge
	EIMSK |= (1 << INT0);	// Interrupt on INT0

}

int main(void) {

	cli();
	
	touchInit();
	
	_delay_ms(500);
	
	elInit();
	
	sei();
	
	for (uint8_t x=0; x<80; x++) {
		EL_Pixel(x, x, on);
		EL_Pixel(x, x+80, on);
		EL_Pixel(79-x, x, on);
		EL_Pixel(79-x, x+80, on);
	}

	//EL_CursorXY(0,1);
	//printf("EL160.80.38-SM1");
	
	for (;;);
}

ISR(INT0_vect) {

	uint16_t volatile touchCoord = EL_TouchscreenRead();
	
	elCursorXY(0,3);
	printf("Touch at row:%d, col:%d", (char)(touchCoord >> 8), (char) touchCoord);
	
}
