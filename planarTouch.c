/*********************************************************/
/* Functions for sed1330 Display Controller              */
/*********************************************************/
// *******************************************************
//
// Software library for driving Planar EL160.80.38-SM1
// EL display with integrated touch screen and SED1330
// display controller.
// 
// Copyright Karl Buchka (kbuchka@gmail.com) for the ATmega168.
//
// Based on Frank Wallenwein's SED1330 library and Philip
// Pemberton's reverse engineering efforts 
// (http://www.philpem.me.uk/elec/lcd/planar160x80/).
// 
//
// *******************************************************
// This software (planarTouch) is Copyrighted 2012 by Karl Buchka,
// I am releasing it under the terms and conditions of the 
//
// 	 GNU GENERAL PUBLIC LICENSE Version 2, June 1991
//
// or later. A copy off which is included in the file COPYING.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY, to the extent permitted by law; without
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2012 Karl Buchka
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
// NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: Karl Buchka <kbuchka@gmail.com>
//
// Contributors: Frank Wallenwein <fwallenwein@tklinux.de>
//				 Philip Pemberton <philpem.me.uk>
//
// *************************************************************

#include <stdio.h>
#include <math.h>
#include <avr/io.h>
#include "planarTouch.h"
#include <util/delay.h>
#include <string.h>

// *******************************************************
// Function Prototypes 
void elSendByte (unsigned  char value );				// Send Data Byte
void elSendCommand (unsigned  char value );				// Send Command Byte
unsigned char elRead (int command);			// Get Data or cursor information

//*******************************************************
// defines
//*******************************************************

static FILE myStdOut = FDEV_SETUP_STREAM(EL_PrintChar, NULL,_FDEV_SETUP_WRITE);

// *************************************************************
// Sed 1330 Command Set 
// *************************************************************
#define SYSSET		0x40	/* System Set							*/
#define SLEEPIN		0x53	/* Enter standby mode					*/
#define DISPOFF		0x58	/* Display on							*/
#define DISPON		0x59	/* Display off							*/
#define SCROLL		0x44	/* Display Start Addr & Display regions	*/
#define CSRFORM		0x5D	/* Set Cursor Type						*/
#define CGRAM		0x5C	/* Start Addr Char. Generator Ram		*/
#define CSRDIR		0x4C	/* Direction of cursor movement			*/
#define HDOTSCR		0x5A	/* Horizontal Scroll Position			*/
#define OVERLAY		0x5B	/* Display Overlay format				*/
#define CSRW		0x46	/* Set Cursor Address					*/
#define CSRR		0x47	/* Read Cursor Address					*/
#define MWRITE		0x42	/* Write to Display Memory				*/
#define MREAD		0x43	/* Read from Display Memory				*/

#define BUSY		0xFF	// Dummy define for busy flag checking.

/*********************************************************/
/* send Command to Controller                            */
/*********************************************************/
void elSendCommand (unsigned char command) {

	while(elRead(BUSY));			// Check busy flag

	EL_CTRL |= _BV(EL_A0);			// Sending command
	EL_DATA  = command;
	
	// Strobe write
	_delay_us(STROBE_DELAY);
	EL_CTRL &= ~_BV(EL_WR);
	_delay_us(STROBE_DELAY);
	EL_CTRL |=  _BV(EL_WR);

	EL_CTRL &= ~_BV(EL_A0);			// Return control to known state
     
}

/*********************************************************/
/* send byte to controller                               */
/*********************************************************/
void elSendByte (unsigned char value) {

	while(elRead(BUSY));		// Check busy flag
 
	EL_DATA = value;

	// Strobe write
	_delay_us(STROBE_DELAY);
	EL_CTRL &= ~_BV(EL_WR);  
	_delay_us(STROBE_DELAY); 
	EL_CTRL |=  _BV(EL_WR);

}

/*********************************************************/
/* read data, cursor, or busy state from controller      */
/*********************************************************/
unsigned char elRead (int command) {

	switch (command) {
		case MREAD:
			elSendCommand(MREAD);	// Read Data
			EL_CTRL |= _BV(EL_A0);	// Set address line
			break;
		case CSRR:
			elSendCommand(CSRR);	// Read cursor
			EL_CTRL |= _BV(EL_A0);	// Set address line
			break;
		case BUSY:
			break;
		default:
			return 0xFF;			// Error :(
			break;
	}
	
	DDR_DATA  =   0x00;			// Data lines as input
    
    // Strobe read
    _delay_us(STROBE_DELAY);
	EL_CTRL &= ~_BV(EL_RD);	
	_delay_us(STROBE_DELAY);
	unsigned char theByte = PIN_DATA;
	EL_CTRL |= _BV(EL_RD);
	
	EL_CTRL &= ~_BV(EL_A0);			// Return control to known state

	DDR_DATA = 0xFF;			// Data lines as output
	
	if (command == BUSY) {
		return !(theByte & 0x40); // STATUS reg bit 6 is 1 if we're clear to write.
	} else {
		return theByte;
	}

}



/*********************************************************/
/* Reset Controller                                      */
/*********************************************************/
void elReset(void) {

	DDR_DATA  = 0xFF;
	DDR_CTRL |= (1 << EL_RD) | (1 << EL_WR) | (1 << EL_RESET) | (1 << EL_A1) | (1 << EL_A0); // Control pins as outputs

	EL_CTRL |=  _BV(EL_RD);		// Not reading
	EL_CTRL |=  _BV(EL_WR);		// Not writing

	EL_CTRL &= ~(1 << EL_A0);
	EL_CTRL &= ~(1 << EL_A1);

	EL_CTRL &= ~_BV(EL_RESET);	// Assert reset
	_delay_ms(500);				// Wait 500ms       
    EL_CTRL |=  _BV(EL_RESET);

}

/*********************************************************
 Init Controller                                       
 Here we have a couple of param hardcoded
   needs to be changed to calculated values
   in future releases.
   Sorry - up to then you have to check maually
   
*********************************************************/
void elInit(void) {
	
	EL_CTRL |= (1 << EL_RD) | (1 << EL_WR); // Not reading or writing
	
	// Reset Controller
	elReset();

	elSendCommand(SYSSET);
	elSendByte(0x38);					// M0=0 (internal CG ROM), M1=0 (no D6 correction), M2=0 (8px char height), WS=1 (dual panel drive), IV=1
	elSendByte((EL_CHARWIDTH-1)|0x80);	// Char width, WF=1 (two frame AC drive)
	elSendByte(EL_CHARHEIGHT-1);    /* Char height             */
	elSendByte(EL_CHARPERROW-1);    /* Chars per row           */
	elSendByte((EL_CHARPERROW-1)+8);    
	elSendByte((EL_CHARROWS * EL_CHARHEIGHT * 2) - 1);
	elSendByte((32 & 0xff));	// APL	-- number of bytes per display line (low)
	elSendByte((32 >> 8));	// APH	-- number of bytes per display line (high)

	elSendCommand(OVERLAY);
	elSendByte(0x01);                   /* XOR Simple Overlay was 1*/
  
	elSendCommand(SCROLL);
	elSendByte(0x00);             /* Low  Byte Start Text Mem  */
	elSendByte(0x00);             /* High Byte Start Text Mem  */
	elSendByte(EL_RESX-1);    /* Screen Lines              */
	elSendByte(0x00);          /* Low Byte Start Graph Mem  */
	elSendByte(0x10);          /* High Byte Start Graph Mem */
	elSendByte(EL_RESX-1);    /* Screen Lines              */
	elSendByte(0x00);
	elSendByte(0x04);
	elSendByte(0x00);
	elSendByte(0x30);
  
	elSendCommand(CSRFORM);
	elSendByte(0x5D);		// block cursor on
	elSendByte(0x04);
	elSendByte(0x86);

	elSendCommand(HDOTSCR);     /* Display on*/
	elSendByte(0x0);            /* was 16Cursor FLash, first+second screen block on  */
  
	elSendCommand(CSRDIR);     /* No Param = shift right */
  
	elSendCommand(DISPON);     /* Display on*/
	elSendByte(0x14);          /* Do not switch on Cursor ( messes up Graphics ) */

	elClearText();
	elClearGraph();

	stdout = &myStdOut;
 
}

/*********************************************************/
/* Set Graphics Cursor                                    */
/*********************************************************/
void elSetCursor(uint16_t addr) {

	// Command 0x46 -- Cursor Address Write
	elSendCommand(CSRW);
	elSendByte((addr & 0xff));
	elSendByte((addr >> 8) & 0xff);
}

uint8_t elGetCursor(void) {

	uint8_t addr, temp;

	// Command 0x47 -- Cursor Address Read
	temp = elRead(CSRR);
	addr = temp;
	temp = elRead(CSRR);
	addr = addr + (temp << 8);

	return addr;
}

void elCursorXY(int x, int y) {

	uint16_t addr = 0;

	if (x > EL_CHARPERROW) {
		x = EL_CHARPERROW;
	}

	if (y > EL_CHARROWS) {
		y = EL_CHARROWS;
	}

	addr = (y * EL_ADDR_INCR) + x;

	elSetCursor(addr);
}

/*********************************************************/
/* Text Clear Screen                                     */
/*********************************************************/
void elClearText(void) {

	elCursorXY(0,0);
	elSendCommand(DISPOFF);
	elSendCommand(MWRITE);
	for (uint16_t i = 0; i < (EL_ADDR_INCR*EL_CHARROWS); i++) {
		elSendByte(' ');
	}
	
	elCursorXY(0,0);
	elSendCommand(DISPON);
}

/*********************************************************/
/* Graphic Clear Screen                                  */
/*********************************************************/
void elClearGraph(void) {

	uint16_t x;//, xmax;

	// move cursor to starting address of graphics page
	elSetCursor(0x1000);
	
	// clear graphics screen with zeroes
	elSendCommand(MWRITE);
	for (x=0; x<XMAX; x++) {
		elSendByte(0x00);
	}
}

/*********************************************************/
/* Print a null terminated string at x,y                 */
/*********************************************************/
void elWriteStr0(unsigned char *myText) {
	while (*myText)
		EL_Char(*myText++);

}

/*********************************************************/
/* Print a char 						                */
/*********************************************************/
void EL_Char(unsigned char myText) {
	elSendCommand(MWRITE);
	elSendByte(myText);
}

/*********************************************************/
/* Set/Reset Pixel                                       */
/*********************************************************/
void EL_Pixel(int x, int y, drawmode show) {

	uint16_t addr, ch;

	// calculate address
	addr = 0x1000 + (y * EL_ADDR_INCR) + (x / EL_CHARWIDTH);

	// send address
	elSetCursor(addr);
	
	// read current pixel state
	ch = elRead(MREAD);
	if ( (show == on) || (show == fill) ) {
		ch |= 1<<(7-(x % EL_CHARWIDTH));
	} else {
		ch &= ~(1<<(7-(x % EL_CHARWIDTH)));
	}

	// send address again
	elSetCursor(addr);
	
	// update pixel state
	elSendCommand(MWRITE);
	elSendByte(ch);
}


/****************************************************************************/
/* Draws a rectangle from x1,y1 to x2,y2.                                   */
/* Thank you  Knut Baardsen @ Baardsen Software, Norway http://www.baso.no  */
/****************************************************************************/
void EL_Rectangle(int x1,int y1,int x2,int y2,drawmode show) {

	int i;
	for (i=x1; i<=x2; i++) 
		EL_Pixel(i,y1, show);
	for (i=x1; i<=x2; i++) 
		EL_Pixel(i,y2, show); 
	for (i=y1; i<=y2; i++) 
		EL_Pixel(x1,i, show);
	for (i=y1; i<=y2; i++) 
		EL_Pixel(x2,i, show);
	if ( (show == fill)||(show == clear) ) {
		for (i=y1; i<=y2; i++) 
			EL_Line(x1,i,x2,i, show);
	} 
}

/****************************************************************************/
/* Draws a line from x,y at given degree from inner_radius to outer_radius. */
/* Thank you  Knut Baardsen @ Baardsen Software, Norway http://www.baso.no  */
/****************************************************************************/
void EL_DegreeLine(int x,int y, int degree, int inner_radius, int outer_radius, drawmode show) {

	int fx,fy,tx,ty;
	fx = x + (inner_radius * sin(degree * 3.14 / 180));
	fy = y - (inner_radius * cos(degree * 3.14 / 180));
	tx = x + (outer_radius * sin(degree * 3.14 / 180));
	ty = y - (outer_radius * cos(degree * 3.14 / 180));
	EL_Line(fx,fy,tx,ty,show);
}

/****************************************************************************/
/* Draws a circle with center at x,y with given radius.                     */
/* Set show to 1 to draw pixel, set to 0 to hide pixel.                     */
/* Thank you  Knut Baardsen @ Baardsen Software, Norway http://www.baso.no  */
/****************************************************************************/
void EL_Circle(int x, int y, int radius, drawmode  show) {

	int xc = 0;
	int yc = radius;
	int p = 3 - (radius<<1);
	while (xc <= yc)   {
		EL_Pixel(x + xc, y + yc, show);
		EL_Pixel(x + xc, y - yc, show);
		EL_Pixel(x - xc, y + yc, show);
		EL_Pixel(x - xc, y - yc, show);
		EL_Pixel(x + yc, y + xc, show);
		EL_Pixel(x + yc, y - xc, show);
		EL_Pixel(x - yc, y + xc, show);
		EL_Pixel(x - yc, y - xc, show);
		if (p < 0) {
			p += (xc++ << 2) + 6;
		} else {
			p += ((xc++ - yc--)<<2) + 10;
		}
	}
}


/****************************************************************************/
/* Draws a line from x1,y1 go x2,y2. Line can be drawn in any direction.    */
/* Thank you  Knut Baardsen @ Baardsen Software, Norway http://www.baso.no  */
/****************************************************************************/
void EL_Line(int x1, int y1, int x2, int y2, drawmode show)  {

	int dy = y2 - y1;
	int dx = x2 - x1;
	int stepx, stepy, fraction;
	if (dy < 0)  {
		dy = -dy;
		stepy = -1;
	} else {
		stepy = 1;
		}
	if (dx < 0) {
		dx = -dx;
		stepx = -1;
	} else {
		stepx = 1;
	}
	
	dy <<= 1;
	dx <<= 1;
	EL_Pixel(x1,y1,show);

	if (dx > dy) {
		fraction = dy - (dx >> 1); 
		while (x1 != x2) {
			if (fraction >= 0) {
				y1 += stepy;
				fraction -= dx;
			}
			
			x1 += stepx;
			fraction += dy;  
			EL_Pixel(x1,y1,show);
		}
		
	} else {
		fraction = dx - (dy >> 1);
		while (y1 != y2) {
			if (fraction >= 0) {
				x1 += stepx;
				fraction -= dy;
			}

			y1 += stepy;
			fraction += dx;
			EL_Pixel(x1,y1,show);
		}
	}
}



/*********************************************************/
/* Print Char ( used for STDIO in avr-libc )             */
/*********************************************************/

static int EL_PrintChar(char myChar, FILE *stream) {  

	stream = NULL;
	EL_Char(myChar);
	return 0;
}

/*********************************************************/
/* Read touchscreen, returns row and col concatenated to a 16bit int */
/*********************************************************/
uint16_t EL_TouchscreenRead(void) {

	uint8_t col, row;

	// set address pins to read column address
	EL_CTRL |= (1 << EL_A1);
	EL_CTRL &= ~(1 << EL_A0);
	
	DDR_DATA  =   0x00;			// Data lines as input
	//EL_DATA = 0xFF;				// Pullups
    
    // Initiate read
    _delay_us(STROBE_DELAY);
	EL_CTRL &= ~(1 << EL_RD);
	_delay_us(STROBE_DELAY);
	
	// Get column
	col = (PIN_DATA & 0x3F);
	
	// Change address pins (without changing RD)
	EL_CTRL |= (1 << EL_A1) | (1 << EL_A0);
	_delay_us(STROBE_DELAY);

	// Get row
	row = (PIN_DATA & 0x07);
	_delay_us(STROBE_DELAY);
	
	// Done reading
	EL_CTRL |= (1 << EL_RD);

	EL_CTRL &= ~(1 << EL_A1);
	EL_CTRL &= ~(1 << EL_A0);
	
	//EL_DATA = 0x00;				// No pullups
	DDR_DATA = 0xFF;			// Data lines as output

	// was there a touch?
	if ((col == 0x3F) && (row == 0x07)) {
		// no touch
		return 0;
	} else {
		// find first '1' bit in column addr
		col = ffs(col);

		// do the same for the row address
		row = ffs(row);

		// if either row or col is out of range, something went wrong
		if ((row > 3) || (col > 6)) {
			return 0;
		} else {
			return (row << 8) + col;
		}
	}

}

