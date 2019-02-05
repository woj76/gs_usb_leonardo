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

#ifndef GS_USB_H
#define GS_USB_H

#define GS_USB_INTERFACE		0
#define GS_USB_ENDPOINT_IN		1
#define GS_USB_ENDPOINT_OUT		2

#define USB_CANDLELIGHT_VENDOR_ID	0x1209
#define USB_CANDLELIGHT_PRODUCT_ID	0x2323
#define USB_GSUSB_VENDOR_ID		0x1D50
#define USB_GSUSB_PRODUCT_ID		0x606F

#define GS_USB_BREQ_HOST_FORMAT		0
#define GS_USB_BREQ_BITTIMING		1
#define GS_USB_BREQ_MODE		2
#define GS_USB_BREQ_BERR		3 // unused by the Linux gs_usb driver
#define GS_USB_BREQ_BT_CONST		4
#define GS_USB_BREQ_DEVICE_CONFIG	5
#define GS_USB_BREQ_TIMESTAMP		6 // unused by the Linux gs_usb driver
#define GS_USB_BREQ_IDENTIFY		7

#define GS_CAN_MODE_RESET		0
#define GS_CAN_MODE_START		1

#define GS_CAN_MODE_NORMAL		0x00
#define GS_CAN_MODE_LISTEN_ONLY		0x01
#define GS_CAN_MODE_LOOP_BACK		0x02
#define GS_CAN_MODE_TRIPLE_SAMPLE	0x04
#define GS_CAN_MODE_ONE_SHOT		0x08

#define GS_CAN_FEATURE_LISTEN_ONLY	0x01
#define GS_CAN_FEATURE_LOOP_BACK	0x02
#define GS_CAN_FEATURE_TRIPLE_SAMPLE	0x04
#define GS_CAN_FEATURE_ONE_SHOT		0x08
#define GS_CAN_FEATURE_HW_TIMESTAMP	0x10 // unused by the Linux gs_usb drivers
#define GS_CAN_FEATURE_IDENTIFY		0x20

#define GS_CAN_IDENTIFY_OFF		0
#define GS_CAN_IDENTIFY_ON		1

#define GS_CAN_FLAG_OVERFLOW		1

typedef struct  {
	uint32_t feature;
	uint32_t fclk_can;
	uint32_t tseg1_min;
	uint32_t tseg1_max;
	uint32_t tseg2_min;
	uint32_t tseg2_max;
	uint32_t sjw_max;
	uint32_t brp_min;
	uint32_t brp_max;
	uint32_t brp_inc;
} gs_device_bt_const;

typedef struct  {
	uint8_t reserved1;
	uint8_t reserved2;
	uint8_t reserved3;
	uint8_t icount;
	uint32_t sw_version;
	uint32_t hw_version;
} gs_device_config;

typedef struct {
	uint32_t byte_order;
} gs_host_config;

typedef struct {
	uint32_t prop_seg;
	uint32_t phase_seg1;
	uint32_t phase_seg2;
	uint32_t sjw;
	uint32_t brp;
} gs_device_bittiming;

typedef struct {
	uint32_t mode;
} gs_identify_mode;

typedef struct {
	uint32_t mode;
	uint32_t flags;
} gs_device_mode;

typedef struct {
	uint32_t echo_id;
	uint32_t can_id;
	uint8_t can_dlc;
	uint8_t channel;
	uint8_t flags;
	uint8_t reserved;
	uint8_t data[8];
} gs_host_frame;

extern volatile gs_device_bittiming gs_requested_bittiming;
extern volatile uint8_t gs_can_mode;
extern volatile uint8_t gs_can_mode_flags;

void gs_usb_init();
uint8_t gs_usb_descriptor();
uint8_t gs_usb_setup();

#endif
