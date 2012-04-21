#ifndef PLANARTOUCH_H
#define PLANARTOUCH_H
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
// Board Hardware defines
//
// Please check your Board <-> Display connections 
// and edit Data Port, Control Port, and INT defines.
// The code below, when unmodified, has the data lines
// connected to Port B, the control lines on Port C,
// and the /INT line connected to INT0 (PD2).
//
// *************************************************************
#define EL_DATA			PORTB	/* EL Data Port, Port B */
#define PIN_DATA		PINB      
#define DDR_DATA		DDRB

#define EL_CTRL			PORTC	/* Control Port, Port C */
#define PIN_CTRL		PINC
#define DDR_CTRL		DDRC

#define EL_RD			0		// WR Pin 
#define EL_WR			1		// WD Pin 
#define EL_RESET		2		// Reset Pin 
#define EL_A1			3		// A1 Pin   
#define EL_A0			4		// A0 Pin

#define INT_PIN			INT0	/* Touch screen interrupt. Must be either INT0 or INT1 */

#define STROBE_DELAY	1		// Delay (in uS) for strobing data in and out.

#define EL_RESX			160		/* EL X resolution */
#define EL_RESY			80		/* EL Y resolution */
#define EL_CHARPERROW	27		/* maximum is ( EL_RESX / EL_CHARWIDTH ) */
#define EL_CHARROWS		10		/* should be calculated from RESY/CHARHEIGHT */
#define EL_CHARWIDTH	6		/* = EL_RESX / EL_CHARPERROW */
#define EL_CHARHEIGHT	8
#define EL_ADDR_INCR 32

#define XMAX (EL_ADDR_INCR * EL_CHARROWS * EL_CHARHEIGHT)


// *************************************************************
// Types 
// *************************************************************
typedef enum DrawMode {
	on,    // Set Pixel
	off,   // Clear Pixel 
	fill,  // fill Circel, rectangle etc.
	clear  // clear circle, rectangle etc.
} 
drawmode;


// *************************************************************
// User Functions 
// *************************************************************

void elInit (void);			/* Initialize Display Controler */
void elReset(void);			/* Reset Display Controler.     */
void elClearText(void);		/* Clear Text Screen    */
void elWriteStr0(unsigned char *myText);
void elCursorXY(int x, int y);
void elChar(unsigned char myChar);
static int elPrintChar(char myChar, FILE *stream);	/* for using STDIO in avr-libc */

void elClearGraph(void);							/* Clear Graphic Screen */
void elPixel (int x,int y, drawmode show);
void elLine  (int x1, int y1, int x2, int y2, drawmode show);
void elCircle(int x, int y, int radius, drawmode show);
void elRectangle (int x1,int y1,int x2,int y2,drawmode show);
void elDegreeLine(int x,int y, int degree, int inner_radius, int outer_radius, drawmode show);
uint16_t elTouchscreenRead(void);

#endif
