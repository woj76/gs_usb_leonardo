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

/* This file provides the general USB functionality, yet modified to be
   optimal in the gs_usb context  */

#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "bool.h"
#include "usb.h"
#include "leds.h"

usb_device_configuration* udc;

usb_setup received_setup;
uint16_t sent_control = 0;

// See comments below in the code for why we do not use this part
//volatile uint8_t usb_configuration = 0;
//volatile uint8_t usb_status = 0;
//volatile uint8_t usb_suspended = FALSE;

volatile uint8_t write_blinks = 0;
volatile uint8_t read_blinks = 0;

inline uint8_t usb_receive(uint8_t *ptr, uint8_t len) {
	uint8_t r = TRUE;
	register uint8_t _sreg = SREG;
	cli();
	UENUM = udc->usb_endpoint_out;
	if(UEINTX & (1<<RWAL)) { // alternatively UEINTX & (1<<RXOUTI)
		UEINTX = ~(1<<RXOUTI);
		while (len--) {
			*ptr++ = UEDATX;
		}
		// This check is not really needed in the gs_usb scenario
		// if(!(UEINTX & (1<<RWAL))) // alternatively !UEBCLX)
		UEINTX &= ~(1 << FIFOCON);
		write_blinks = NUM_BLINKS;
	} else {
		r = FALSE;
	}
	SREG = _sreg;
	return r;
}

inline void usb_send(uint8_t* ptr, uint8_t len, uint8_t blink) {
//	if (usb_suspended) {
//		UDCON |= (1 << RMWKUP);
//	}
	uint16_t time_out = 0xFFFF;
	register uint8_t _sreg = SREG;
	cli();
	UENUM = udc->usb_endpoint_in;
	// It seems that because of the double USB buffer in the gs_usb scenario
	// this check always immediatelly goes through, thus the time out check does
	// not cost extra cycles, yet it is useful to handle disconnected cable and
	// similar situations.
	while(!(UEINTX & (1<<RWAL))) { // alternatively !(UEINTX & (1<<TXINI))
		if(!time_out--) {
			SREG = _sreg;
			return;
		}
	}
	UEINTX = ~(1<<TXINI);
	while (len--) {
		UEDATX = *ptr++;
	}
	UEINTX &= ~(1 << FIFOCON);
	SREG = _sreg;
	// We blink selectively for gs_usb, that is only for the actual CAN received or error
	// frames and not for the echo / confirmation ones
	if(blink) {
		read_blinks = NUM_BLINKS;
	}
}

inline void init_endpoint(uint8_t index, uint8_t type, uint8_t size) {
	UENUM = index;
	UECONX = (1<<EPEN);
	UECFG0X = type;
	UECFG1X = size;
}

uint8_t usb_send_control8(uint8_t d) {
	if (sent_control++ < received_setup.wLength) {
		while (!(UEINTX & ((1<<TXINI)|(1<<RXOUTI))));
		if (UEINTX & (1<<RXOUTI)) {
			return FALSE;
		}
		UEDATX = d;
		if (!(sent_control & 0x3F)) {
			UEINTX = ~(1<<TXINI);
		}
	}
	return TRUE;
}

uint8_t usb_send_control(const void* d, uint8_t len) {
	while (len--) {
		if (!usb_send_control8(pgm_read_byte(d++))) {
			return FALSE;
		}
	}
	return TRUE;
}

uint8_t usb_send_string(const uint8_t* d, uint8_t len) {
	uint8_t r = usb_send_control8(2 + len*2);
	r &= usb_send_control8(USB_STRING_DESCRIPTOR_TYPE);
	for(uint8_t i = 0; i < len; i++) {
		r &= usb_send_control8(pgm_read_byte(d++));
		r &= usb_send_control8(0);
	}
	return r;
}

void usb_receive_control(void* d, uint8_t len) {
	while (!(UEINTX & (1<<RXOUTI)));
        uint8_t* ptr = (uint8_t*)d;
	while (len--) {
		*ptr++ = UEDATX;
	}
	UEINTX = ~(1<<RXOUTI);
}

void usb_init(usb_device_configuration* device_configuration) {
	udc = device_configuration;
	IDENTIFY_LED_MODE;
	READY_LED_MODE;
	READ_LED_MODE;
	WRITE_LED_MODE;
	IDENTIFY_LED_OFF;
	READY_LED_OFF;
	READ_LED_OFF;
	WRITE_LED_OFF;
//	usb_configuration = 0;
//	usb_suspended = FALSE;
//	usb_status = 0;
	UHWCON |= (1<<UVREGE);
	PLLCSR |= (1<<PINDIV);
	PLLCSR |= (1<<PLLE);
	while (!(PLLCSR & (1<<PLOCK)));
	USBCON |= (1<<USBE);
	USBCON |= (1<<OTGPADE);
//	USBCON |= (1<<VBUSTE);
	UDCON &= ~((1<<RSTCPU) | (1<<LSM) | (1<<RMWKUP));
//	UDIEN = (1<<EORSTE) | (1<<SOFE) | (1<<SUSPE);
	UDIEN = (1<<EORSTE) | (1<<SOFE);
//	while(!(USBSTA & (1<<VBUS)));
	USBCON &= ~(1<<FRZCLK);
	UDCON &= ~(1<<DETACH);
}

ISR(USB_COM_vect) {
	UENUM = 0;
	if (!(UEINTX & (1<<RXSTPI))) {
		return;
	}
	sent_control = 0;

	uint8_t *ptr = (uint8_t *)&received_setup;
	for(uint8_t i = 0; i<8; i++) {
		*ptr++ = UEDATX;
	}
	UEINTX = ~((1<<RXSTPI) | (1<<RXOUTI) | (1<<TXINI));

	uint8_t t = received_setup.bmRequestType;
	if (t & REQUEST_DEVICETOHOST) {
		while (!(UEINTX & (1<<TXINI)));
	} else {
		UEINTX = ~(1<<TXINI);
	}

	uint8_t res = FALSE;
	if ((t & REQUEST_TYPE) == REQUEST_STANDARD) {
		uint8_t r = received_setup.bRequest;
// The commented out code is something that you would find in USBCore.cpp and should probably 
// be here to get a properly behaving USB device. However, the gs_usb driver does not seem to 
// need any of this...
//		uint16_t wValue = received_setup.wValueL | (received_setup.wValueH << 8);
//		if (r == GET_STATUS) {
//			if (t == (REQUEST_DEVICETOHOST | REQUEST_STANDARD | REQUEST_DEVICE)) {
//				UEDATX = usb_status; UEDATX = 0;
//			} else {
//				UEDATX = 0; UEDATX = 0;
//			}
//			res = TRUE;
//		} else if (r == CLEAR_FEATURE) {
//			if((t == (REQUEST_HOSTTODEVICE | REQUEST_STANDARD | REQUEST_DEVICE))
//				&& (wValue == DEVICE_REMOTE_WAKEUP)) {
//				usb_status &= ~FEATURE_REMOTE_WAKEUP_ENABLED;
//				res = TRUE;
//			}
//		} else if (r == SET_FEATURE) {
//			if((t == (REQUEST_HOSTTODEVICE | REQUEST_STANDARD | REQUEST_DEVICE))
//				&& (wValue == DEVICE_REMOTE_WAKEUP)) {
//				usb_status |= FEATURE_REMOTE_WAKEUP_ENABLED;
//				res = TRUE;
//			}
//		} else 
		if (r == SET_ADDRESS) {
			while (!(UEINTX & (1<<TXINI)));
			UDADDR = received_setup.wValueL | (1<<ADDEN);
			res = TRUE;
//		} else if (r == GET_CONFIGURATION) {
//			UEDATX = 1;
//			res = TRUE;
		} else if (r == GET_DESCRIPTOR) {
			res = (*udc->usb_descriptor_func)(&received_setup);
		} else if (r == SET_CONFIGURATION && (t & REQUEST_RECIPIENT) == REQUEST_DEVICE) {
			READY_LED_ON;
			init_endpoint(1, EP_TYPE_BULK_IN, EP_DOUBLE_64);
			init_endpoint(2, EP_TYPE_BULK_OUT, EP_DOUBLE_64);
			UERST = 0x7E;
			UERST = 0;
			// usb_configuration = received_setup.wValueL;
			res = TRUE;
		}
	} else if(received_setup.wIndex == udc->usb_interface_num) {
		res = (*udc->usb_setup_func)(&received_setup);
	}

	if (res) {
		UEINTX = ~(1<<TXINI);
	} else {
		UECONX = (1<<STALLRQ) | (1<<EPEN);
	}
}

// Some references for using the VBUS state bit, none of this seemed to work as expected though, neither 
// the supspending and clock on-off switching suggested in the ATMega 32U4 docs. Another way of checking
// for active USB connection is checking UDFNUML as in the newer USBCore.cpp implementation (frame counting),
// but this requires delays which we do not want here.
//
// USBCON.VBUSTE - enable vbus transition interrupt
// USBSTA.VBUS - check vbus state
// USBINT.VBUSTI - interrupt flag, clear by software

/*
void usb_deinit() {
	usb_configuration = 0;
	usb_status = 0;
	UDCON |= (1<<DETACH);
	USBCON &= ~(1<<USBE);
	PLLCSR &= ~(1<<PLLE);
	UHWCON &= ~(1<<UVREGE);
	(*usb_init_func)();
}
*/

/*
void usb_clock_on() {
	CLKSEL0 |= (1 << EXTE);
	while(!(CLKSTA & (1<<EXTON)));
	CLKSEL0 |= (1 << CLKS);
	PLLCSR |= (1<<PLLE);
	CLKSEL0 &= ~(1 << RCE);
	while (!(PLLCSR & (1<<PLOCK)));
	USBCON &= ~(1<<FRZCLK);
}

void usb_clock_off() {
	USBCON |= (1<<FRZCLK);
	PLLCSR &= ~(1<<PLLE);
	CLKSEL0 |= (1 << RCE);
	while(!(CLKSTA & (1 << RCON)));
	CLKSEL0 &= ~(1 << CLKS);
	CLKSEL0 &= ~(1 << EXTE);
}
*/

uint8_t write_blink_counter = 0;
uint8_t read_blink_counter = 0;

ISR(USB_GEN_vect) {
	// End of reset - initialise things
	if (UDINT & (1<<EORSTI)) {
		UDINT &= ~(1<<EORSTI);
		init_endpoint(0, EP_TYPE_CONTROL, EP_SINGLE_64);
		(*udc->usb_init_func)();
		//usb_configuration = 0;
		UEIENX = (1 << RXSTPE);
	}
	// Start of frame every 1ms - utilise for LED flashing
	if (UDINT & (1<<SOFI)) {
		UDINT &= ~(1<<SOFI);
		if(write_blinks) {
			if(!write_blink_counter) {
				write_blinks--;
				WRITE_LED_FLIP;
			}
			write_blink_counter = (write_blink_counter + 1) & BLINK_TIME;
		}else{
			WRITE_LED_OFF;
		}
		if(read_blinks) {
			if(!read_blink_counter) {
				read_blinks--;
				READ_LED_FLIP;
			}
			read_blink_counter = (read_blink_counter + 1) & BLINK_TIME;
		}else{
			READ_LED_OFF;
		}
	}
// See above, it was impossible to get the datasheet adviced clock switching to work
//	if (UDINT & (1<<WAKEUPI)) {
////		usb_clock_on();
//		usb_suspended = FALSE;
//		UDINT &= ~(1<<WAKEUPI);
//	}else if (UDINT & (1<<SUSPI)) {
//		UDINT &= ~((1<<SUSPI) /*| (1<<WAKEUPI)*/);
////		usb_clock_off();
//		usb_suspended = TRUE;
//	}
}
