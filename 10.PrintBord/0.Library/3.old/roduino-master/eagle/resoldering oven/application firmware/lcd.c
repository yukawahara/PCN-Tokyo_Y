/* Reflow Toaster Oven
 * http://frank.circleofcurrent.com/reflowtoasteroven/
 * Copyright (c) 2011 Frank Zhao
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 * This file contains everything needed to work with the
 * NHD-C160100DiZ LCD being used in the circuit
 * including a set of bare minimum TWI routines to use the I2C bus
 * and a 8x5 pixel font, working with STDIO utilities
 *
 */

#include "lcd.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <util/delay.h>

/*
 * bare minimum I2C routines
*/

void i2c_init()
{
	PORTD |= _BV(0) | _BV(1); // enable internal pull-ups
	DDRD &= ~(_BV(0) | _BV(1)); // enable internal pull-ups
	
	TWSR = 0; // no prescaler => prescaler = 1
	TWBR = ((F_CPU / 100000UL) - 16) / 2; // change the I2C clock rate
	
	// warning, the NHD-C160100DiZ works at 100 KHz only, but the datasheet says 400 KHz, the datasheet is wrong
	
	TWCR = 1<<TWEN;	// enable twi module, no interrupt
}

void i2c_start(uint8_t address, char readwrite)
{
	address <<= 1;
	address |= readwrite ? 1 : 0;
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN); // send REPEAT START condition
	loop_until_bit_is_set(TWCR, TWINT);
	TWDR = address; // send device address
	TWCR = (1<<TWINT) | (1<<TWEN);
}

void i2c_stop()
{
	loop_until_bit_is_set(TWCR, TWINT); // wait for transmission to finish
	TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTO);
	volatile uint16_t timeout = 0;
	do { timeout++; } while (bit_is_set(TWCR, TWSTO) && timeout != 0);
}

void i2c_write(uint8_t data)
{
	loop_until_bit_is_set(TWCR, TWINT); // wait for transmission to finish
	TWDR = data; // set the data to send
	TWCR = (1<<TWINT) | (1<<TWEN); // initiate transmission
}

/*
 * LCD routines for NHD-C160100DiZ
 *
*/

#define LCD_SLAVE_ADDR 0x3F

#define LCD_PORTx_CS PORTD
#define LCD_DDRx_CS DDRD
#define LCD_PIN_CS 6

//#define LCD_PORTx_A0 PORTB
//#define LCD_DDRx_A0 DDRB
//#define LCD_PIN_A0 7

#define LCD_PORTx_RESET PORTD
#define LCD_DDRx_RESET DDRD
#define LCD_PIN_RESET 4

#define LCD_DATASEND 0x40
#define LCD_COMSEND 0x00

#define lcd_delay_ms(x) _delay_ms((x)*2)

uint8_t lcd_cur_column; // track current column
char lcd_draw_started; // used to automatically call lcd_draw_start if needed

void lcd_init()
{
	// setup the pins
	
	LCD_DDRx_CS |= _BV(LCD_PIN_CS);
	//LCD_DDRx_A0 |= _BV(LCD_PIN_A0);
	LCD_DDRx_RESET |= _BV(LCD_PIN_RESET);
	
	LCD_PORTx_CS |= _BV(LCD_PIN_CS);
	//LCD_PORTx_A0 &= ~_BV(LCD_PIN_A0);
	
	i2c_init();
	
	// perform reset
	LCD_PORTx_RESET &= ~_BV(LCD_PIN_RESET);
	_delay_ms(10);
	LCD_PORTx_RESET |= _BV(LCD_PIN_RESET);
	_delay_ms(10);
	
	// initialize according to example code
	// I didn't really look at exactly what this is doing
	
	i2c_start(LCD_SLAVE_ADDR, 0);
	
	i2c_write(LCD_COMSEND);
	i2c_write(0x48);
	i2c_write(0x64);
	i2c_write(0xA0);
	i2c_write(0xC8);
	i2c_write(0x44);
	i2c_write(0x00);
	i2c_write(0xAB);
	i2c_write(0x26);
	i2c_write(0x81);
	i2c_write(0x15);
	i2c_write(0x56);
	i2c_write(0x64);
	lcd_delay_ms(2);
	i2c_write(0x2C);
	i2c_write(0x66);
	lcd_delay_ms(2);
	i2c_write(0x2E);
	lcd_delay_ms(2);
	i2c_write(0x2F);
	i2c_write(0xF3);
	i2c_write(0x00);
	i2c_write(0x96);
	i2c_write(0x38);
	i2c_write(0x75);
	i2c_write(0x97);
	
	// this computes greyscale settings
	for (int i = 0x80, j = 0, k = 0; i <= 0xBF; i += 4, j++)
	{
		k = j * 4;
		for (int m = 0; m < 4; m++)
		{
			i2c_write(i + m);
			i2c_write(k);
		}
	}
	
	i2c_write(0x38);
	i2c_write(0x74);
	i2c_write(0xAF);
	i2c_stop();
	lcd_delay_ms(2);
	
	lcd_cur_column = 0;
	lcd_draw_started = 0;
}

void lcd_set_row(uint8_t r)
{
	if (lcd_draw_started != 0)
	{
		lcd_draw_end();
	}
	
	i2c_start(LCD_SLAVE_ADDR, 0);
	i2c_write(LCD_COMSEND);
	i2c_write(r | 0xB0);
	i2c_stop();
}

void lcd_set_column(uint8_t c)
{
	if (lcd_draw_started != 0)
	{
		lcd_draw_end();
	}

	lcd_cur_column = c;
	
	i2c_start(LCD_SLAVE_ADDR, 0);
	i2c_write(LCD_COMSEND);
	//c += 1;
	i2c_write(((c & 0xF0) >> 4) | 0x10);
	i2c_write(((c & 0x0F) >> 0) | 0x00);
	i2c_stop();
}

// slightly faster
void lcd_set_row_column(uint8_t r, uint8_t c)
{
	if (lcd_draw_started != 0)
	{
		lcd_draw_end();
	}
	
	lcd_cur_column = c;
	
	i2c_start(LCD_SLAVE_ADDR, 0);
	i2c_write(LCD_COMSEND);
	i2c_write(r | 0xB0);
	//c += 1;
	i2c_write(((c & 0xF0) >> 4) | 0x10);
	i2c_write(((c & 0x0F) >> 0) | 0x00);
	i2c_stop();
}

void lcd_draw_start()
{
	//LCD_PORTx_A0 |= _BV(LCD_PIN_A0);
	i2c_start(LCD_SLAVE_ADDR, 0);
	i2c_write(LCD_DATASEND);
	
	lcd_draw_started = 1;
}

void lcd_draw_end()
{
	if (lcd_draw_started)
	{
		i2c_stop();
	}
	
	//LCD_PORTx_A0 &= ~_BV(LCD_PIN_A0);
	
	lcd_draw_started = 0;
}

void lcd_draw_unit(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3)
{
	if (lcd_draw_started == 0) lcd_draw_start();
	
	i2c_write(b0);
	i2c_write(b1);
	i2c_write(b2);
	i2c_write(b3);
	lcd_cur_column++;
}

void lcd_clear_row(char r, char c_start)
{
	lcd_set_row_column(r, c_start);
	lcd_clear_restofrow();
}

void lcd_clear_restofrow()
{
	for (int i = lcd_cur_column; i < LCD_WIDTH; i++)
	{
		lcd_draw_unit(0, 0, 0, 0);
	}
	lcd_draw_end();
}

void lcd_clear_screen()
{
	for (int r = 0; r < LCD_ROWS; r++)
	{
		lcd_clear_row(r, 0);
	}
}

// this font is a 8x5 pixel font, this array is generated using another tool I wrote
uint8_t PROGMEM lcd_font[] = {
	// SPACE (0x20)
	0x00, 0x00, 0x00, 0x00, 0x00,
	// ! (0x21)
	0x00, 0x00, 0xBE, 0x00, 0x00, 
	// " (0x22)
	0x00, 0x06, 0x00, 0x06, 0x00, 
	// # (0x23)
	0x28, 0xFE, 0x28, 0xFE, 0x28, 
	// $ (0x24)
	0x08, 0x54, 0xFE, 0x54, 0x20, 
	// % (0x25)
	0x23, 0x13, 0x08, 0x64, 0x62, 
	// &amp; (0x26)
	0x6C, 0x92, 0xAA, 0x44, 0xA0, 
	// ' (0x27)
	0x00, 0x02, 0x04, 0x08, 0x00, 
	// ( (0x28)
	0x00, 0x38, 0x44, 0x82, 0x00, 
	// ) (0x29)
	0x00, 0x82, 0x44, 0x38, 0x00, 
	// * (0x2A)
	0x28, 0x10, 0x7C, 0x10, 0x28, 
	// + (0x2B)
	0x10, 0x10, 0x7C, 0x10, 0x10, 
	// , (0x2C)
	0x00, 0xB0, 0x70, 0x00, 0x00, 
	// - (0x2D)
	0x10, 0x10, 0x10, 0x10, 0x10, 
	// . (0x2E)
	0x00, 0x60, 0x60, 0x00, 0x00, 
	// / (0x2F)
	0x40, 0x20, 0x10, 0x08, 0x04, 
	// 0 (0x30)
	0x7C, 0xA2, 0x92, 0x8A, 0x7C, 
	// 1 (0x31)
	0x00, 0x84, 0xFE, 0x80, 0x00, 
	// 2 (0x32)
	0x84, 0xC2, 0xA2, 0x92, 0x8C, 
	// 3 (0x33)
	0x44, 0x82, 0x92, 0x92, 0x6C, 
	// 4 (0x34)
	0x30, 0x28, 0x24, 0xFE, 0x20, 
	// 5 (0x35)
	0x4E, 0x8A, 0x8A, 0x8A, 0x72, 
	// 6 (0x36)
	0x7C, 0x92, 0x92, 0x92, 0x64, 
	// 7 (0x37)
	0x02, 0x02, 0xF2, 0x0A, 0x06, 
	// 8 (0x38)
	0x6C, 0x92, 0x92, 0x92, 0x6C, 
	// 9 (0x39)
	0x0C, 0x92, 0x92, 0x52, 0x3C, 
	// : (0x3A)
	0x00, 0x6C, 0x6C, 0x00, 0x00, 
	// ; (0x3B)
	0x00, 0xAC, 0x6C, 0x00, 0x00, 
	// &lt; (0x3C)
	0x10, 0x28, 0x44, 0x82, 0x00, 
	// = (0x3D)
	0x28, 0x28, 0x28, 0x28, 0x28, 
	// &gt; (0x3E)
	0x00, 0x82, 0x44, 0x28, 0x10, 
	// ? (0x3F)
	0x04, 0x02, 0xA2, 0x12, 0x0C, 
	// @ (0x40)
	0x7C, 0x82, 0xBA, 0xAA, 0xBC, 
	// A (0x41)
	0xF8, 0x24, 0x22, 0x24, 0xF8, 
	// B (0x42)
	0xFE, 0x92, 0x92, 0x92, 0x6C, 
	// C (0x43)
	0x7C, 0x82, 0x82, 0x82, 0x44, 
	// D (0x44)
	0xFE, 0x82, 0x82, 0x44, 0x38, 
	// E (0x45)
	0xFE, 0x92, 0x92, 0x92, 0x82, 
	// F (0x46)
	0xFE, 0x12, 0x12, 0x12, 0x02, 
	// G (0x47)
	0x7C, 0x82, 0x92, 0x92, 0x74, 
	// H (0x48)
	0xFE, 0x10, 0x10, 0x10, 0xFE, 
	// I (0x49)
	0x00, 0x82, 0xFE, 0x82, 0x00, 
	// J (0x4A)
	0x40, 0x80, 0x82, 0x7E, 0x02, 
	// K (0x4B)
	0xFE, 0x10, 0x28, 0x44, 0x82, 
	// L (0x4C)
	0xFE, 0x80, 0x80, 0x80, 0x80, 
	// M (0x4D)
	0xFE, 0x04, 0x08, 0x04, 0xFE, 
	// N (0x4E)
	0xFE, 0x08, 0x10, 0x20, 0xFE, 
	// O (0x4F)
	0x7C, 0x82, 0x82, 0x82, 0x7C, 
	// P (0x50)
	0xFE, 0x12, 0x12, 0x12, 0x0C, 
	// Q (0x51)
	0x7C, 0x82, 0xA2, 0x42, 0xBC, 
	// R (0x52)
	0xFE, 0x12, 0x32, 0x52, 0x8C, 
	// S (0x53)
	0x8C, 0x92, 0x92, 0x92, 0x62, 
	// T (0x54)
	0x02, 0x02, 0xFE, 0x02, 0x02, 
	// U (0x55)
	0x7E, 0x80, 0x80, 0x80, 0x7E, 
	// V (0x56)
	0x3E, 0x40, 0x80, 0x40, 0x3E, 
	// W (0x57)
	0x7E, 0x80, 0x7C, 0x80, 0x7E, 
	// X (0x58)
	0xC6, 0x28, 0x10, 0x28, 0xC6, 
	// Y (0x59)
	0x0E, 0x10, 0xE0, 0x10, 0x0E, 
	// Z (0x5A)
	0xC2, 0xA2, 0x92, 0x8A, 0x86, 
	// [ (0x5B)
	0x00, 0xFF, 0x81, 0x81, 0x00, 
	// \ (0x5C)
	0x04, 0x08, 0x10, 0x20, 0x40, 
	// ] (0x5D)
	0x00, 0x82, 0x82, 0xFE, 0x00, 
	// ^ (0x5E)
	0x08, 0x04, 0x02, 0x04, 0x08, 
	// _ (0x5F)
	0x80, 0x80, 0x80, 0x80, 0x80, 
	// ` (0x60) used as degree symbol
	//0x00, 0x0E, 0x16, 0x00, 0x00, 
	0x00, 0x0E, 0x0A, 0x0E, 0x00, 
	// a (0x61)
	0x40, 0xA8, 0xA8, 0xA8, 0xF0, 
	// b (0x62)
	0xFE, 0x90, 0x88, 0x88, 0x70, 
	// c (0x63)
	0x70, 0x88, 0x88, 0x88, 0x00, 
	// d (0x64)
	0x70, 0x88, 0x88, 0x90, 0xFE, 
	// e (0x65)
	0x70, 0xA8, 0xA8, 0xA8, 0x30, 
	// f (0x66)
	0x10, 0xFC, 0x12, 0x02, 0x04, 
	// g (0x67)
	0x0C, 0x92, 0x92, 0x92, 0x7E, 
	// h (0x68)
	0xFE, 0x20, 0x10, 0x10, 0xF0, 
	// i (0x69)
	0x00, 0x88, 0xFA, 0x80, 0x00, 
	// j (0x6A)
	0x40, 0x80, 0x88, 0x7A, 0x00, 
	// k (0x6B)
	0xFE, 0x20, 0x50, 0x88, 0x00, 
	// l (0x6C)
	0x00, 0x82, 0xFE, 0x80, 0x00, 
	// m (0x6D)
	0xF8, 0x08, 0x30, 0x08, 0xF0, 
	// n (0x6E)
	0xF8, 0x10, 0x08, 0x08, 0xF0, 
	// o (0x6F)
	0x70, 0x88, 0x88, 0x88, 0x70, 
	// p (0x70)
	0xF8, 0x28, 0x28, 0x28, 0x10, 
	// q (0x71)
	0x18, 0x24, 0x24, 0x28, 0xFC, 
	// r (0x72)
	0xF8, 0x10, 0x08, 0x08, 0x10, 
	// s (0x73)
	0x90, 0xA8, 0xA8, 0xA8, 0x40, 
	// t (0x74)
	0x08, 0x7E, 0x88, 0x80, 0x40, 
	// u (0x75)
	0x78, 0x80, 0x80, 0x40, 0xF8, 
	// v (0x76)
	0x38, 0x40, 0x80, 0x40, 0x38, 
	// w (0x77)
	0x78, 0x80, 0x60, 0x80, 0x78, 
	// x (0x78)
	0x88, 0x58, 0x20, 0xD0, 0x88, 
	// y (0x79)
	0x98, 0xA0, 0xA0, 0xA0, 0x78, 
	// z (0x7A)
	0x88, 0xC8, 0xA8, 0x98, 0x88, 
	// { (0x7B)
	0x00, 0x10, 0x6C, 0x82, 0x00, 
	// | (0x7C)
	0x00, 0x00, 0xEE, 0x00, 0x00, 
	// } (0x7D)
	0x00, 0x82, 0x6C, 0x10, 0x00, 
	// ~ (0x7E)
	0x04, 0x02, 0x04, 0x08, 0x04, 
};

void lcd_draw_char(char c)
{
	if (c == '\n')
	{
		// blank out rest of line if '\n'
		lcd_clear_restofrow();
	}
	else if (c == '\r')
	{
		// do nothing, just in case of a mistake in the code
	}
	else if (c == '\t')
	{
		// tab, so make a single space first...
		lcd_draw_unit(0, 0, 0, 0);
		lcd_draw_unit(0, 0, 0, 0);
		lcd_draw_unit(0, 0, 0, 0);
		lcd_draw_unit(0, 0, 0, 0);
		lcd_draw_unit(0, 0, 0, 0);
		
		// then try to align it with other tabs
		do
		{
			lcd_draw_unit(0, 0, 0, 0);
		}
		while ((lcd_cur_column % (FONT_WIDTH * FONT_TAB_SIZE)) != 0);
	}
	else if (c != 0)
	{
		// print the character using the font stored within flash memory
		for (int i = 0; i < FONT_WIDTH-1; i++)
		{
			// each vertical line at a time
			uint8_t f = pgm_read_byte(&(lcd_font[(c - ' ') * 5 + i]));
			lcd_draw_unit(f, f, f, f);
		}
	
		lcd_draw_unit(0, 0, 0, 0); // this makes at least one blank vertical pixel between each character
		// this makes each character 6 pixels wide, hence why FONT_WIDTH is 6 and not 5
	}
}

// used to stream with STDIO
static int lcd_draw_putchar(char c, FILE* f)
{
	lcd_draw_char(c);
	return 0;
}

FILE lcd_stream = FDEV_SETUP_STREAM(lcd_draw_putchar, NULL, _FDEV_SETUP_WRITE);