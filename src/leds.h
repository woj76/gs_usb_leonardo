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

#ifndef LEDS_H
#define LEDS_H

#include <avr/io.h>

// PD7
#define POWER_LED_MODE		(DDRD |= 0x80)
#define POWER_LED_ON		(PORTD |= 0x80)
#define POWER_LED_OFF		(PORTD &= 0x7F)
#define POWER_LED_FLIP		(PORTD ^= 0x80)

// PD4
#define IDENTIFY_LED_MODE	(DDRD |= 0x10)
#define IDENTIFY_LED_ON		(PORTD |= 0x10)
#define IDENTIFY_LED_OFF	(PORTD &= 0xEF)
#define IDENTIFY_LED_FLIP	(PORTD ^= 0x10)

// PF0 - This is the built-in LED
#define READY_LED_MODE		(DDRF |= 0x01)
#define READY_LED_ON		(PORTF |= 0x01)
#define READY_LED_OFF		(PORTF &= 0xFE)
#define READY_LED_FLIP		(PORTF ^= 0x01)

// PF4
#define WRITE_LED_MODE		(DDRF |= 0x10)
#define WRITE_LED_ON		(PORTF |= 0x10)
#define WRITE_LED_OFF		(PORTF &= 0xEF)
#define WRITE_LED_FLIP		(PORTF ^= 0x10)

// PD6
#define READ_LED_MODE		(DDRD |= 0x40)
#define READ_LED_ON		(PORTD |= 0x40)
#define READ_LED_OFF		(PORTD &= 0xBF)
#define READ_LED_FLIP		(PORTD ^= 0x40)

#endif
