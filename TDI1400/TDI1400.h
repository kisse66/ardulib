/*
	Simple TDIx-1400 LED display driver

	4 x 7-digit LED display with serial interface. Did not have full datasheet,
	so some of the bit shifting is based on trial and error.

	This is a simple library using slow pin control, but can be generic
	(one display update takes about 1.2ms on 16MHz mega328).

    --------------------------------------------------------------------------
	Copyright (C) 2018 Krister W. <kisse66@hobbylabs.org>

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation and/or other
	materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
	BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
	HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
	THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdint.h>

#define TDI1400_LED1	0x01	// pin 10
#define TDI1400_COLON	0x02	// middle colon
#define TDI1400_LED2	0x04	// pin 11
#define TDI1400_LED3	0x08	// pin 12
#define TDI1400_LED4	0x10	// pin 9
#define TDI1400_LED5	0x20	// pin 8

// comment to reduce code size if only numbers (or own patterns) needed
#define TDI1400_STRINGS

class TDI1400
{
public:
	TDI1400(uint8_t en, uint8_t ck, uint8_t d);

	void begin();			// initialize driver and pins
	void clear();			// clear display
	void update();			// display current data buffer
	void blank(bool on=true);	// blank leading zero digits if true
	void hex(uint16_t num, uint8_t start=0, uint8_t len=4);	// write 16-bit hex number to buffer
	void udec(uint16_t num, uint8_t start = 0, uint8_t len = 4);	// write 16-bit number to buffer
	void dec(int16_t num, uint8_t start = 0, uint8_t len = 4);	// write 16-bit signed number to buffer
	void digit(uint8_t digit, uint8_t nibble);	// write given nibble 0-F to given digit 0-3
	void rawwrite(uint8_t digit, uint8_t segs);	// update given digit raw data a buffer
	void led(uint8_t leds); // bitmask for external LEDs

	void printch(char ch, uint8_t digit=0);
	void print(const char *str, uint8_t start=0, uint8_t len=4);	// print a string (some characters not supported=blank)

protected:

	uint16_t bcd(uint16_t num) const;

	uint8_t m_enpin;
	uint8_t m_ckpin;
	uint8_t m_dpin;
	uint8_t m_disp[4];
	bool m_blank;
	uint8_t m_leds;
};
