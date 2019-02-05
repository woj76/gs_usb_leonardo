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

#include <avr/pgmspace.h>

#include "bool.h"
#include "usb.h"
#include "gs_usb.h"
#include "leds.h"

/* This file provides the GS specific USB functionality */

const uint8_t STRING_LANGUAGE[] PROGMEM = { 0x04, 0x03, 0x09, 0x04};
const uint8_t STRING_PRODUCT[] PROGMEM = "Arduino Leonardo";
const uint8_t STRING_MANUFACTURER[] PROGMEM = "Arduino LLC";
const uint8_t STRING_SERIAL[] PROGMEM = "USB-CAN";

/* USB device identifier with vendor and product id set so that the Linux
   gs_usb driver can recognise it. */
const usb_device_descriptor device_descriptor PROGMEM = {
	.len = USB_DEVICE_DESC_SIZE,
	.dtype = USB_DEVICE_DESCRIPTOR_TYPE,
	.usbVersion = USB_VERSION,
	.deviceClass = 0x00,
	.deviceSubClass = 0x00,
	.deviceProtocol = 0x00,
	.packetSize0 = USB_EP_SIZE,
	.idVendor = USB_CANDLELIGHT_VENDOR_ID,
	.idProduct = USB_CANDLELIGHT_PRODUCT_ID,
	.deviceVersion = 0x100,
	.iManufacturer = IMANUFACTURER,
	.iProduct = IPRODUCT,
	.iSerialNumber = ISERIAL,
	.bNumConfigurations = 1
};

const usb_config_descriptor device_config PROGMEM = {
	.len = USB_CONFIGUARTION_DESC_SIZE,
	.dtype = USB_CONFIGURATION_DESCRIPTOR_TYPE,
	.clen = USB_CONFIGUARTION_DESC_SIZE + USB_INTERFACE_DESC_SIZE + 2*USB_ENDPOINT_DESC_SIZE,
	.numInterfaces = 1,
	.config = 1,
	.iconfig = 0,
	.attributes = USB_CONFIG_BUS_POWERED | USB_CONFIG_REMOTE_WAKEUP,
	.maxPower = USB_CONFIG_POWER_MA(500)
};

const usb_interface_descriptor gs_usb_interface PROGMEM = {
	.len = USB_INTERFACE_DESC_SIZE,
	.dtype = USB_INTERFACE_DESCRIPTOR_TYPE,
	.number = 0,
	.alternate = 0,
	.numEndpoints = 2,
	.interfaceClass = USB_DEVICE_CLASS_VENDOR_SPECIFIC,
	.interfaceSubClass = USB_DEVICE_SUBCLASS_VENDOR_SPECIFIC,
	.protocol = USB_PROTOCOL_VENDOR_SPECIFIC,
	.iInterface = 0
};

const usb_endpoint_descriptor gs_usb_endpoint_in PROGMEM = {
	.len = USB_ENDPOINT_DESC_SIZE,
	.dtype = USB_ENDPOINT_DESCRIPTOR_TYPE,
	.addr = USB_ENDPOINT_IN(GS_USB_ENDPOINT_IN),
	.attr = USB_ENDPOINT_TYPE_BULK,
	.packetSize = USB_EP_SIZE,
	.interval = 0x00
};

const usb_endpoint_descriptor gs_usb_endpoint_out PROGMEM = {
	.len = USB_ENDPOINT_DESC_SIZE,
	.dtype = USB_ENDPOINT_DESCRIPTOR_TYPE,
	.addr = USB_ENDPOINT_OUT(GS_USB_ENDPOINT_OUT),
	.attr = USB_ENDPOINT_TYPE_BULK,
	.packetSize = USB_EP_SIZE,
	.interval = 0x00
};

const gs_device_bt_const GS_DEVICE_BT_CONST PROGMEM = {
	.feature =
		GS_CAN_FEATURE_LISTEN_ONLY |
		GS_CAN_FEATURE_LOOP_BACK |
		GS_CAN_FEATURE_IDENTIFY |
		GS_CAN_FEATURE_TRIPLE_SAMPLE |
		GS_CAN_FEATURE_ONE_SHOT,
	.fclk_can = 8000000, // I really thought this ought to be 16MHz...
	.tseg1_min = 3, .tseg1_max = 8 /* was 16 */, // Max should be 8 according to the chip docs, also it does not list 3 as minimum
	.tseg2_min = 2, .tseg2_max = 8,
	.sjw_max = 4,
	.brp_min = 1,
	.brp_max = 64,
	.brp_inc = 1
};

/* This is copied from candleLight_fw sources without too much thought... */
const gs_device_config GS_DEVICE_CONFIG PROGMEM = {
	.reserved1 = 0,
	.reserved2 = 0,
	.reserved3 = 0,
	.icount = 0,
	.sw_version = 2,
	.hw_version = 1
};

volatile gs_device_bittiming gs_requested_bittiming;
volatile uint8_t gs_can_mode = GS_CAN_MODE_RESET;
volatile uint8_t gs_can_mode_flags = GS_CAN_MODE_NORMAL;

union received_control_t {
	gs_host_config host_config;
	gs_device_bittiming device_bittiming;
	gs_identify_mode identify_mode;
	gs_device_mode device_mode;
} received_control;

void gs_usb_init() {
	gs_can_mode = GS_CAN_MODE_RESET;
	gs_can_mode_flags = GS_CAN_MODE_NORMAL;
}

uint8_t gs_usb_descriptor(usb_setup* setup) {
	uint8_t t = setup->wValueH;
	if (t == USB_DEVICE_DESCRIPTOR_TYPE) {
		return usb_send_control(&device_descriptor, sizeof(usb_device_descriptor));
	} else if (t == USB_CONFIGURATION_DESCRIPTOR_TYPE) {
		t = usb_send_control(&device_config, sizeof(usb_config_descriptor));
		t &= usb_send_control(&gs_usb_interface, sizeof(usb_interface_descriptor));
		t &= usb_send_control(&gs_usb_endpoint_in, sizeof(usb_endpoint_descriptor));
		t &= usb_send_control(&gs_usb_endpoint_out, sizeof(usb_endpoint_descriptor));
		return t;
	} else if (t == USB_STRING_DESCRIPTOR_TYPE) {
		t = setup->wValueL;
		if (t == 0) {
			return usb_send_control(STRING_LANGUAGE, sizeof(STRING_LANGUAGE));
		} else if (t == IPRODUCT) {
			return usb_send_string(STRING_PRODUCT, sizeof(STRING_PRODUCT));
		} else if (t == IMANUFACTURER) {
			return usb_send_string(STRING_MANUFACTURER, sizeof(STRING_MANUFACTURER));
		} else if (t == ISERIAL) {
			return usb_send_string(STRING_SERIAL, sizeof(STRING_SERIAL));
		}
	}
	return FALSE;
}

uint8_t gs_usb_setup(usb_setup* setup) {

	uint8_t r = setup->bRequest;
	uint8_t t = setup->bmRequestType;

	if (t == REQUEST_DEVICETOHOST_VENDOR_INTERFACE) {
		if(r == GS_USB_BREQ_BT_CONST) {
			return usb_send_control(&GS_DEVICE_BT_CONST, sizeof(GS_DEVICE_BT_CONST));
		} else if(r == GS_USB_BREQ_DEVICE_CONFIG) {
			return usb_send_control(&GS_DEVICE_CONFIG, sizeof(GS_DEVICE_CONFIG));
		}
	}else if (t == REQUEST_HOSTTODEVICE_VENDOR_INTERFACE) {
		if(r == GS_USB_BREQ_HOST_FORMAT) {
			usb_receive_control(&received_control.host_config, sizeof(gs_host_config));
			// Sanity check, probably can be skipped
			if(received_control.host_config.byte_order == 0x0000beef) {
				return TRUE;
			}
		}else if(r == GS_USB_BREQ_BITTIMING) {
			usb_receive_control(&received_control.device_bittiming, sizeof(gs_device_bittiming));
			gs_requested_bittiming = received_control.device_bittiming;
			return TRUE;
		}else if(r == GS_USB_BREQ_MODE) {
			usb_receive_control(&received_control.device_mode, sizeof(gs_device_mode));
			gs_can_mode = received_control.device_mode.mode;
			gs_can_mode_flags = received_control.device_mode.flags;
			return TRUE;
		}else if(r == GS_USB_BREQ_IDENTIFY) {
			usb_receive_control(&received_control.identify_mode, sizeof(gs_identify_mode));
			if(received_control.identify_mode.mode == GS_CAN_IDENTIFY_ON) {
				IDENTIFY_LED_ON;
				return TRUE;
			}else if(received_control.identify_mode.mode == GS_CAN_IDENTIFY_OFF) {
				IDENTIFY_LED_OFF;
				return TRUE;
			}
		}
	}
	return FALSE;
}

