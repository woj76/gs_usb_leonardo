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

#ifndef USB_H
#define USB_H

#include <stdint.h>

#define BLINK_TIME		0x3F	// 64 ms
#define NUM_BLINKS		6	// Flash LEDs minimally 6/2=3 times for any bulk transfer activity

#define GET_STATUS		0
#define CLEAR_FEATURE		1
#define SET_FEATURE		3
#define SET_ADDRESS		5
#define GET_DESCRIPTOR		6
#define SET_DESCRIPTOR		7
#define GET_CONFIGURATION	8
#define SET_CONFIGURATION	9
#define GET_INTERFACE		10
#define SET_INTERFACE		11

#define REQUEST_HOSTTODEVICE	0x00
#define REQUEST_DEVICETOHOST	0x80
#define REQUEST_DIRECTION	0x80

#define REQUEST_STANDARD	0x00
#define REQUEST_CLASS		0x20
#define REQUEST_VENDOR		0x40
#define REQUEST_TYPE		0x60

#define REQUEST_DEVICE		0x00
#define REQUEST_INTERFACE	0x01
#define REQUEST_ENDPOINT	0x02
#define REQUEST_OTHER		0x03
#define REQUEST_RECIPIENT	0x03

#define REQUEST_DEVICETOHOST_VENDOR_INTERFACE	0xC1	// (REQUEST_DEVICETOHOST | REQUEST_VENDOR | REQUEST_INTERFACE)
#define REQUEST_HOSTTODEVICE_VENDOR_INTERFACE	0x41	// (REQUEST_HOSTTODEVICE | REQUEST_VENDOR | REQUEST_INTERFACE)
#define REQUEST_DEVICETOHOST_CLASS_INTERFACE	0xA1	// (REQUEST_DEVICETOHOST | REQUEST_CLASS | REQUEST_INTERFACE)
#define REQUEST_HOSTTODEVICE_CLASS_INTERFACE	0x21	// (REQUEST_HOSTTODEVICE | REQUEST_CLASS | REQUEST_INTERFACE)
#define REQUEST_DEVICETOHOST_STANDARD_INTERFACE	0x81	// (REQUEST_DEVICETOHOST | REQUEST_STANDARD | REQUEST_INTERFACE)

#define USB_DEVICE_DESC_SIZE			18
#define USB_CONFIGUARTION_DESC_SIZE		9
#define USB_INTERFACE_DESC_SIZE			9
#define USB_ENDPOINT_DESC_SIZE			7

#define USB_DEVICE_DESCRIPTOR_TYPE		1
#define USB_CONFIGURATION_DESCRIPTOR_TYPE	2
#define USB_STRING_DESCRIPTOR_TYPE		3
#define USB_INTERFACE_DESCRIPTOR_TYPE		4
#define USB_ENDPOINT_DESCRIPTOR_TYPE		5

#define DEVICE_REMOTE_WAKEUP		1

#define FEATURE_SELFPOWERED_ENABLED	0x01
#define FEATURE_REMOTE_WAKEUP_ENABLED	0x02

//#define USB_DEVICE_CLASS_COMMUNICATIONS		0x02
//#define USB_DEVICE_CLASS_HUMAN_INTERFACE	0x03
//#define USB_DEVICE_CLASS_STORAGE		0x08
#define USB_DEVICE_CLASS_VENDOR_SPECIFIC	0xFF
#define USB_DEVICE_SUBCLASS_VENDOR_SPECIFIC	0xFF
#define USB_PROTOCOL_VENDOR_SPECIFIC		0xFF

#define USB_CONFIG_POWERED_MASK		0x40
#define USB_CONFIG_BUS_POWERED		0x80
#define USB_CONFIG_SELF_POWERED		0xC0
#define USB_CONFIG_REMOTE_WAKEUP	0x20

#define USB_CONFIG_POWER_MA(mA)		((mA)/2)

#define USB_ENDPOINT_OUT(addr)		(addr)
#define USB_ENDPOINT_IN(addr)		(addr | 0x80)

#define USB_ENDPOINT_TYPE_MASK		0x03
#define USB_ENDPOINT_TYPE_CONTROL	0x00
#define USB_ENDPOINT_TYPE_ISOCHRONOUS	0x01
#define USB_ENDPOINT_TYPE_BULK		0x02
#define USB_ENDPOINT_TYPE_INTERRUPT	0x03

#define USB_VERSION		0x200

#define USB_EP_SIZE		64
#define EP_SINGLE_64		0x32
#define EP_DOUBLE_64		0x36

#define EP_TYPE_CONTROL		(0x00)
#define EP_TYPE_BULK_IN		((1<<EPTYPE1) | (1<<EPDIR))
#define EP_TYPE_BULK_OUT	(1<<EPTYPE1)

#define IMANUFACTURER		1
#define IPRODUCT		2
#define ISERIAL			3

typedef struct {
	uint8_t len;
	uint8_t dtype;
	uint16_t usbVersion;
	uint8_t	deviceClass;
	uint8_t	deviceSubClass;
	uint8_t	deviceProtocol;
	uint8_t	packetSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t deviceVersion;
	uint8_t	iManufacturer;
	uint8_t	iProduct;
	uint8_t	iSerialNumber;
	uint8_t	bNumConfigurations;
} usb_device_descriptor;

typedef struct {
	uint8_t	len;
	uint8_t	dtype;
	uint16_t clen;
	uint8_t	numInterfaces;
	uint8_t	config;
	uint8_t	iconfig;
	uint8_t	attributes;
	uint8_t	maxPower;
} usb_config_descriptor;

typedef struct {
	uint8_t len;
	uint8_t dtype;
	uint8_t number;
	uint8_t alternate;
	uint8_t numEndpoints;
	uint8_t interfaceClass;
	uint8_t interfaceSubClass;
	uint8_t protocol;
	uint8_t iInterface;
} usb_interface_descriptor;

typedef struct {
	uint8_t len;
	uint8_t dtype;
	uint8_t addr;
	uint8_t attr;
	uint16_t packetSize;
	uint8_t interval;
} usb_endpoint_descriptor;

typedef struct {
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint8_t wValueL;
	uint8_t wValueH;
	uint16_t wIndex;
	uint16_t wLength;
} usb_setup;

typedef struct {
	void (*usb_init_func)();
	uint8_t (*usb_descriptor_func)(usb_setup* setup);
	uint8_t (*usb_setup_func)(usb_setup* setup);
	uint8_t usb_interface_num;
	uint8_t usb_endpoint_in;
	uint8_t usb_endpoint_out;
} usb_device_configuration;

void usb_init(usb_device_configuration* device_configuration);
uint8_t usb_send_control(const void* d, uint8_t len);
uint8_t usb_send_string(const uint8_t* d, uint8_t len);
void usb_receive_control(void* d, uint8_t len);
void usb_send(uint8_t* ptr, uint8_t len, uint8_t blink);
uint8_t usb_receive(uint8_t* ptr, uint8_t len);

#endif
