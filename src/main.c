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

/* The main top-level functionality for gs_usb_leonardo */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "usb.h"
#include "gs_usb.h"
#include "mcp.h"
#include "mcp_gs.h"
#include "bool.h"
#include "leds.h"

#define HOST_FRAME_OUT_1_IDX	0
#define HOST_FRAME_OUT_2_IDX	1
#define HOST_FRAME_IN_IDX	2
#define HOST_FRAME_IN_NUM	MCP_N_TXBUFFERS
#define HOST_FRAME_ERR_IDX	(HOST_FRAME_IN_IDX + HOST_FRAME_IN_NUM)
#define HOST_FRAMES_SIZE	(HOST_FRAME_ERR_IDX + 1)

volatile gs_host_frame host_frames[HOST_FRAMES_SIZE];

uint8_t mcp_index;
volatile uint8_t mcp_free[MCP_N_TXBUFFERS];

usb_device_configuration gs_udc = {
	.usb_init_func = gs_usb_init,
	.usb_descriptor_func = gs_usb_descriptor,
	.usb_setup_func = gs_usb_setup,
	.usb_interface_num = GS_USB_INTERFACE,
	.usb_endpoint_in = GS_USB_ENDPOINT_IN,
	.usb_endpoint_out = GS_USB_ENDPOINT_OUT
};

void clear_data() {
	for(uint8_t i=0; i<HOST_FRAMES_SIZE; i++) {
		host_frames[i].echo_id = 0;
		host_frames[i].can_id = 0;
		host_frames[i].can_dlc = 0;
		host_frames[i].channel = 0;
		host_frames[i].flags = 0;
		host_frames[i].reserved = 0;
		for(uint8_t j=0; j<8; j++) {
			host_frames[i].data[j] = 0;
		}
	}
	// These three freames are the actual frames (non echo).
	host_frames[HOST_FRAME_OUT_1_IDX].echo_id =
		host_frames[HOST_FRAME_OUT_2_IDX].echo_id =
		host_frames[HOST_FRAME_ERR_IDX].echo_id =
		0xFFFFFFFF;
	mcp_free[0] = mcp_free[1] = mcp_free[2] = TRUE;
	mcp_index = 0;
}

ISR(INT6_vect) {
	uint8_t ri = mcp_service_interrupt();
	if(ri & MCP_RX0IF) {
		mcp_to_gs_host_frame(mcp_buf_in[0], &host_frames[HOST_FRAME_OUT_1_IDX]);
		usb_send((uint8_t *)&host_frames[HOST_FRAME_OUT_1_IDX], sizeof(gs_host_frame), TRUE);
	}
	if(ri & MCP_RX1IF) {
		mcp_to_gs_host_frame(mcp_buf_in[1], &host_frames[HOST_FRAME_OUT_2_IDX]);
		usb_send((uint8_t *)&host_frames[HOST_FRAME_OUT_2_IDX], sizeof(gs_host_frame), TRUE);
	}
	if(ri & MCP_TX0IF) {
		usb_send((uint8_t *)&host_frames[HOST_FRAME_IN_IDX], sizeof(gs_host_frame), FALSE);
		mcp_free[0] = TRUE;
	}
	if(ri & MCP_TX1IF) {
		usb_send((uint8_t *)&host_frames[HOST_FRAME_IN_IDX + 1], sizeof(gs_host_frame), FALSE);
		mcp_free[1] = TRUE;
	}
	if(ri & MCP_TX2IF) {
		usb_send((uint8_t *)&host_frames[HOST_FRAME_IN_IDX + 2], sizeof(gs_host_frame), FALSE);
		mcp_free[2] = TRUE;
	}
	if(ri & MCP_ERRIF) {
		mcp_to_err_host_frame(mcp_err_flags, &host_frames[HOST_FRAME_ERR_IDX]);
		usb_send((uint8_t *)&host_frames[HOST_FRAME_ERR_IDX], sizeof(gs_host_frame), TRUE);
	}
}

void main_loop() {
main_loop_repeat:
	if(!gs_can_mode) {
		return;
	}
	if(mcp_free[mcp_index]) {
		uint8_t hf_index = mcp_index + HOST_FRAME_IN_IDX;
		if(usb_receive((uint8_t *)&host_frames[hf_index], sizeof(gs_host_frame))) {
			mcp_free[mcp_index] = FALSE;
			mcp_enqueue_can_frame(mcp_index, gs_host_frame_to_mcp(&host_frames[hf_index], mcp_buf_out));
			mcp_index++;
			if(mcp_index == MCP_N_TXBUFFERS) {
				mcp_index = 0;
			}

		}
	}
	goto main_loop_repeat;
}

/* A separate communication loop for the loopback mode. The main problem it
   seems is that the MCP looses some incoming CAN frame interrupts when in
   loopback mode. The fact that there are three transmit buffers, but only two
   receive ones does not help. So, to make it work the frames are sent with an
   immediate  wait for acknowledgement using only one buffer, and the received
   ones are manually polled for (they seem to arrive immediatelly). Yet, when
   the usb_send is called too quickly after the one that sends back the echo
   frame, things lock down (not sure at which end). Hence the manual delay that
   seems to solve the issue. This contraption is acceptable considering
   loopback mode is typically used for checking devices connectivity only. But
   note that performance wise (bus speed) this is not optimal. */
void loopback_main_loop() {
loopback_main_loop_repeat:
	if(!gs_can_mode) {
		return;
	}
	if(usb_receive((uint8_t *)&host_frames[HOST_FRAME_IN_IDX], sizeof(gs_host_frame))) {
		mcp_enqueue_can_frame(0, gs_host_frame_to_mcp(&host_frames[HOST_FRAME_IN_IDX], mcp_buf_out));
		if(mcp_send_can_frame(0) == OK) {
			usb_send((uint8_t *)&host_frames[HOST_FRAME_IN_IDX], sizeof(gs_host_frame), FALSE);
		}
	}
	// TODO Why is this delay necessary?
	// 0xC0 is not the smallest value that works, but safe
	uint16_t tc = 0xC0;
	while(--tc) {
		asm volatile("nop");
	}
	uint8_t r = mcp_receive_can_frame();
	if(r) {
		r--;
		mcp_to_gs_host_frame(mcp_buf_in[r], &host_frames[r]);
		usb_send((uint8_t *)&host_frames[r], sizeof(gs_host_frame), TRUE);
	}
	goto loopback_main_loop_repeat;
}

void main() {
	sei();
	usb_init(&gs_udc);
	DDRE &= 0xBF; // MCP interrupt pin
	EICRB = (EICRB & ~((1<<ISC60) | (1<<ISC61))) | (2 << ISC60);
	POWER_LED_MODE;
	POWER_LED_ON;
repeat_main:
	clear_data();
	EIMSK &= ~(1<<INT6);
	mcp_set_mode_normal();
	while(!gs_can_mode);
	if(gs_can_mode_flags & GS_CAN_MODE_LOOP_BACK) {
		mcp_set_mode_loopback();
	} else {
		EIMSK |= (1<<INT6);
		if(gs_can_mode_flags & GS_CAN_MODE_LISTEN_ONLY) {
			mcp_set_mode_listen();
		}
	}
	gs_bittiming_to_mcp(&gs_requested_bittiming, gs_can_mode_flags & GS_CAN_MODE_TRIPLE_SAMPLE, mcp_cnfs);
	if(mcp_begin(FALSE) == OK && mcp_mode_one_shot(gs_can_mode_flags & GS_CAN_MODE_ONE_SHOT) == OK) {
		if(gs_can_mode_flags & GS_CAN_MODE_LOOP_BACK) {
			loopback_main_loop();
		} else {
			main_loop();
		}
	}
	goto repeat_main;
}
