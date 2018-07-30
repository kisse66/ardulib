#include <TDI1400.h>

#include "Arduino.h"

// 7-segment table
static const uint8_t dispdata[] = {
	0b11111100,	// 0 ASCII 48
	0b01100000, // 1
	0b11011010, // 2
	0b11110010, // 3
	0b01100110, // 4
	0b10110110, // 5
	0b10111110, // 6
	0b11100000, // 7
	0b11111110,	// 8
	0b11100110, // 9  ASCII 57

	0b11101110, // A  ASCII 65
	0b00111110, // B
	0b10011100, // C
	0b01111010, // D
	0b10011110, // E
	0b10001110, // F
#ifdef TDI1400_STRINGS
	0b11110110, // G
	0b01101110,	// H
	0b01100000, // I
	0b01111000, // J
	0b00000000, // K (NO)
	0b00011100, // L
	0b11101100, // M
	0b00101010, // N
	0b00111010, // O
	0b11001110, // P
	0b00000000, // Q (NO)
	0b00001010, // R
	0b10110110, // S
	0b11100000, // T
	0b01111100, // U
	0b00000000, // V (NO)
	0b00000000, // W (NO)
	0b00000000, // X (NO)
	0b01110110, // Y
	0b00000000, // Z (NO) ASCII 90
	// TODO consider moving the special chars here from print()?
#endif
};



TDI1400::TDI1400(uint8_t en, uint8_t ck, uint8_t d) : 	m_enpin(en), m_ckpin(ck), m_dpin(d), m_blank(false), m_leds(0)
{

}

void TDI1400::begin()
{
	digitalWrite(m_enpin, 1);
	digitalWrite(m_ckpin, 1);
	pinMode(m_ckpin, OUTPUT);
	pinMode(m_dpin, OUTPUT);
	pinMode(m_enpin, OUTPUT);
	clear();
	update();
}

void TDI1400::clear()
{
	m_disp[0] = 0;
	m_disp[1] = 0;
	m_disp[2] = 0;
	m_disp[3] = 0;
}

void TDI1400::update()
{
	uint8_t i;

	digitalWrite(m_enpin, 0);

	// shift in 40 zeros to re-sync
	digitalWrite(m_dpin, 0);
	for (i = 0; i < 40; i++) {
		digitalWrite(m_ckpin, 0);
		digitalWrite(m_ckpin, 1);
	}

	// write one '1', will latch the data when shifted out (?)
	digitalWrite(m_dpin, 1);
	digitalWrite(m_ckpin, 0);
	digitalWrite(m_ckpin, 1);

	// 4 digits
	bool blank = m_blank;
	for (uint8_t d = 0; d < 4; d++) {
		// digit data
		uint8_t dd = m_disp[d];
		if (blank && dd == dispdata[0] && d!=3)	// blank leading zero
			dd = 0;
		else
			blank = false;

		if ( (m_leds & TDI1400_LED1) && (d == 0) ||
		     (m_leds & TDI1400_COLON) && (d == 1) ||
			 (m_leds & TDI1400_LED2) && (d == 2) ||
			 (m_leds & TDI1400_LED3) && (d == 3) )
			dd |= 1;

		for (i = 0; i < 8; i++) {
			digitalWrite(m_dpin, (dd & 0x80) ? 1 : 0);
			digitalWrite(m_ckpin, 0);
			dd <<= 1;
			digitalWrite(m_ckpin, 1);
		}

	} // digit

	// and then last eLEDs bits + 1 for 34+1 bits total
	digitalWrite(m_dpin, (m_leds & TDI1400_LED4) ? 1 : 0);
	digitalWrite(m_ckpin, 0);
	digitalWrite(m_ckpin, 1);
	digitalWrite(m_dpin, (m_leds & TDI1400_LED5) ? 1 : 0);
	digitalWrite(m_ckpin, 0);
	digitalWrite(m_ckpin, 1);
	digitalWrite(m_dpin,0);
	digitalWrite(m_ckpin, 0);
	digitalWrite(m_ckpin, 1);

	digitalWrite(m_enpin, 1);
}
void TDI1400::blank(bool on)
{
	m_blank = on;
}

void TDI1400::hex(uint16_t num, uint8_t start, uint8_t len)
{
	uint8_t nibble;

	if (start < 1) {
		nibble = (num >> 12) & 0x0F;
		m_disp[0] = dispdata[nibble];
		len--;
	}
	if (start < 2 && len) {
		nibble = (num >> 8) & 0x0F;
		m_disp[1] = dispdata[nibble];
		len--;
	}
	if (start < 3 && len) {
		nibble = (num >> 4) & 0x0F;
		m_disp[2] = dispdata[nibble];
		len--;
	}
	if (start < 4 && len) {
		nibble = num & 0x000F;
		m_disp[3] = dispdata[nibble];
	}
}

void TDI1400::led(uint8_t leds)
{
	m_leds = leds;
}

void TDI1400::udec(uint16_t num, uint8_t start, uint8_t len)
{
	hex(bcd(num),start,len);
}

void TDI1400::dec(int16_t num, uint8_t start, uint8_t len)
{
	if (num < 0) {
		m_disp[start] = 0b00000010; // -
		start++;
		len--;
		num *= -1;
	}
	hex(bcd(num), start, len);
}

void TDI1400::digit(uint8_t digit, uint8_t nibble)
{
	rawwrite(digit, dispdata[nibble & 0xF]);
}

void TDI1400::rawwrite(uint8_t digit, uint8_t segs)
{
	if (digit < 4)
		m_disp[digit] = segs;
}

uint16_t TDI1400::bcd(uint16_t num) const
{
	uint16_t shift = 0;
	uint16_t digit;
	uint16_t bcd = 0;
	while (num > 0) {
		digit = num % 10;
		bcd += (digit << shift);
		shift += 4;
		num /= 10;
	}

	return bcd;
}

void TDI1400::print(const char *str, uint8_t start, uint8_t len)
{
#ifdef TDI1400_STRINGS
	for(uint8_t i=0; i<len; i++) {
		if(!(*str))
			return;
		printch(*str, start+i);
		str++;
	}
#endif
}

void TDI1400::printch(char ch, uint8_t digit)
{
#ifdef TDI1400_STRINGS
	if (digit>3)
		return;

	if (ch == 32) { // space
		m_disp[digit] = 0;
	} else
	if (ch == 45) { // minus
		m_disp[digit] = 0b00000010;
	} else
	if (ch == 95) { // underscore
		m_disp[digit] = 0b00010000;
	} else
	if (ch > 47 && ch < 58 ) { // 0-9
		m_disp[digit] = dispdata[ch -'0'];
	} else
	if (ch > 64 && ch < 91 ) { // A-Z
		m_disp[digit] = dispdata[ch -'A' + 10];
	}
#endif
}