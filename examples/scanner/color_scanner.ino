/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 Modified by Pedro Valverde <geek@4dmania.com> 31/12/2017-12 to
 support ansi escape color codes and provide an enhanced visual
 interface through a serial terminal that support 16 color modes
 like Atom IDE or Putty

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Channel scanner
 *
 * Example to detect interference on the various channels available.
 * This is a good diagnostic tool to check whether you're picking a
 * good channel for your application.
 *
 * Inspired by cpixip.
 * See http://arduino.cc/forum/index.php/topic,54795.0.html
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

#define ESC_COLOR_FG(fg)				"\x1b[" fg "m"
#define ESC_COLOR_BG(bg)				"\x1b[" bg "m"
#define ESC_COLOR(fg, bg)				"\x1b[" fg ";" bg "m"
#define ESC_COLOR_RESET					"\x1b[0m"

#define ESC_COLOR_FG_BLACK				"30"
#define ESC_COLOR_FG_RED				"31"
#define ESC_COLOR_FG_GREEN				"32"
#define ESC_COLOR_FG_YELLOW				"33"
#define ESC_COLOR_FG_BLUE				"34"
#define ESC_COLOR_FG_MAGENTA			"35"
#define ESC_COLOR_FG_CYAN				"36"
#define ESC_COLOR_FG_WHITE				"37"
#define ESC_COLOR_FG_BRIGHT_BLACK		"90"
#define ESC_COLOR_FG_BRIGHT_RED			"91"
#define ESC_COLOR_FG_BRIGHT_GREEN		"92"
#define ESC_COLOR_FG_BRIGHT_YELLOW		"93"
#define ESC_COLOR_FG_BRIGHT_BLUE		"94"
#define ESC_COLOR_FG_BRIGHT_MAGENTA		"95"
#define ESC_COLOR_FG_BRIGHT_CYAN		"96"
#define ESC_COLOR_FG_BRIGHT_WHITE		"97"

#define ESC_COLOR_BG_BLACK				"40"
#define ESC_COLOR_BG_RED				"41"
#define ESC_COLOR_BG_GREEN				"42"
#define ESC_COLOR_BG_YELLOW				"43"
#define ESC_COLOR_BG_BLUE				"44"
#define ESC_COLOR_BG_MAGENTA			"45"
#define ESC_COLOR_BG_CYAN				"46"
#define ESC_COLOR_BG_WHITE				"47"
#define ESC_COLOR_BG_BRIGHT_BLACK		"100"
#define ESC_COLOR_BG_BRIGHT_RED			"101"
#define ESC_COLOR_BG_BRIGHT_GREEN		"102"
#define ESC_COLOR_BG_BRIGHT_YELLOW		"103"
#define ESC_COLOR_BG_BRIGHT_BLUE		"104"
#define ESC_COLOR_BG_BRIGHT_MAGENTA		"105"
#define ESC_COLOR_BG_BRIGHT_CYAN		"106"
#define ESC_COLOR_BG_BRIGHT_WHITE		"107"

#define RF24_CE_PIN					9
#define RF24_CSN_PIN				10
#define RF24_MAX_CHANNELS			127

RF24 radio(RF24_CE_PIN, RF24_CSN_PIN);

void setup()
{
	Serial.begin(57600); printf_begin();

	printf(ESC_COLOR_RESET "\n\rWireless 2.4GHz Carrier Scanner\r\n");

	radio.begin();
	radio.setAutoAck(false);
	radio.startListening();
    radio.stopListening();

	printf("Freq Range: 2.400GHz - 2.526GHz\r\n", 400 + radio.getChannel());
	printf("Model: nRF24L01"); if (radio.isPVariant()) printf("+"); printf("\r\n");
	printf("\r\n");

	int i = 0; while (i < RF24_MAX_CHANNELS)
	{ printf("2.%dGHz           %s", 400 + i, (i)? "  ": ""); i += 20; }
	printf("\r\n");

	i = 0; while (i < RF24_MAX_CHANNELS)
	{ printf("|                    "); i += 20; }
	printf("\r\n");
}

byte values[RF24_MAX_CHANNELS];

void loop()
{
	memset(values, 0, sizeof(values));

	byte counter = 32;
	while (counter--)
	{
		for (byte i = RF24_MAX_CHANNELS; i; i--)
		{
			radio.setChannel(i);
			radio.startListening();
			delayMicroseconds(170);			// RPD measurent (130us + 40us) based on the Nordic nRF24L01+ datasheet specification (page 25)
			if (radio.testCarrier()) values[i]++;
			radio.stopListening();
		}
	}

	// Print out channel measurements in a single hex digit with ansi escape colors.
	byte val, last = 0;
	printf(ESC_COLOR_FG(ESC_COLOR_FG_BRIGHT_BLACK));
	for (byte i = 0; i < RF24_MAX_CHANNELS;)
	{
		val =  (values[i]>0x0F)? 0x0F: values[i];
		if (val != last)
		{
			printf((values[i]>0x17)? ESC_COLOR_FG(ESC_COLOR_FG_BRIGHT_MAGENTA): (values[i]>0x0F)? ESC_COLOR_FG(ESC_COLOR_FG_MAGENTA): (values[i]>0x08)? ESC_COLOR_FG(ESC_COLOR_FG_BRIGHT_RED): (values[i]>0x04)? ESC_COLOR_FG(ESC_COLOR_FG_RED): (values[i]>0x00)? ESC_COLOR_FG(ESC_COLOR_FG_GREEN): ESC_COLOR_FG(ESC_COLOR_FG_BRIGHT_BLACK));
			last = val;
		}
		printf("%X",(values[i]>0x0F)? 0x0F: values[i]&0xF);
		if (!(++i % 20)) printf(" ");
	}
	printf("\r\n");
}