/*

  This file is part of gs_usb_leonardo project --
  gs_usb compatible SocketCAN firmware for Arduino Leonardo /
  MCP2515 based USB device, see

          https://github.com/woj76/gs_usb_leonardo

  For information about how this code came to be and what / whose
  work it is based on, please see the LICENSE.md file in the project
  root directory.

  Copyright (C) 2019 Wojciech Mostowski <wojciech.mostowski@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License, or any later
  version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  The copy of the license can be found in the licenses directory of
  the project, or on GNU/FSF website at https://www.gnu.org/licenses/.

*/

/* Compactified SPI functionality */

#include <avr/interrupt.h>
#include "spi.h"

void spi_init() {
	register uint8_t _sreg = SREG;
	cli();
	PORTB |= 0x01;
	DDRB |= 0x01;
	SPCR = (1 << SPE) | (1 << MSTR) /*| (SPI_MODE0 & SPI_MODE_MASK) */ /* | ((clockDiv >> 1) & SPI_CLOCK_MASK) */;
	SPSR = (0x01 & SPI_2XCLOCK_MASK);
	DDRB |= 0x06;
	SREG = _sreg;
}

inline uint8_t spi_transfer8(uint8_t data) {
	SPDR = data;
	asm volatile("nop");
	while (!(SPSR & (1 << SPIF)));
	return SPDR;
}

inline void spi_transfer(uint8_t *p, uint8_t count) {
	SPDR = *p;
	while (--count) {
		uint8_t out = *(p + 1);
		while (!(SPSR & (1 << SPIF)));
		uint8_t in = SPDR;
		SPDR = out;
		*p++ = in;
	}
	while (!(SPSR & (1 << SPIF)));
	*p = SPDR;
}

