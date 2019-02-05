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

/* The translation bridge between the gs_usb and MCP2515 CAN data formats */ 

#include "mcp_gs.h"
#include "mcp.h"
#include "can.h"

void gs_bittiming_to_mcp(volatile gs_device_bittiming* bittiming, uint8_t triple_sample, uint8_t* cnfs) {
	cnfs[0] = (((uint8_t)bittiming->sjw - 1) << CNF1_SJW_SHIFT) 
		| ((uint8_t)bittiming->brp - 1); // FIXME divide (?) brp by 2 if clk is defined to be 16MHz in the config
	cnfs[1] = BTLMODE | (triple_sample ? SAMPLE_3X : SAMPLE_1X);
	cnfs[1] |= (((uint8_t)bittiming->phase_seg1 - 1) << CNF2_PS1_SHIFT)
		| ((uint8_t)bittiming->prop_seg - 1);
	cnfs[2] = SOF_ENABLE | ((uint8_t)bittiming->phase_seg2 - 1);
}

void mcp_to_gs_host_frame(uint8_t* buf, volatile gs_host_frame* gs_frame) {
	uint32_t id = (buf[0]<<3) + (buf[1]>>5);
	if((buf[1] & MCP_TXB_EXIDE_M) ==  MCP_TXB_EXIDE_M) {
		id = (id<<2) + (buf[1] & 0x03);
		id = (id<<8) + buf[2];
		id = (id<<8) + buf[3];
		id &= CAN_EFF_MASK;
		id |= CAN_EFF_FLAG;
	} else {
		id &= CAN_SFF_MASK;
	}
	if(buf[4] & MCP_RXB_RTR_M) {
		id |= CAN_RTR_FLAG;
	}
	buf[4] &= MCP_DLC_MASK;
	gs_frame->can_id = id;
	gs_frame->can_dlc = buf[4];
	for(uint8_t i = 0; i < buf[4]; i++) {
		gs_frame->data[i] = buf[5+i];
	}
}

uint8_t gs_host_frame_to_mcp(volatile gs_host_frame* gs_frame, uint8_t* buf) {
	uint8_t res = 5;
	uint8_t can_len = gs_frame->can_dlc /*& MCP_DLC_MASK*/;
	for(uint8_t i=0; i<can_len; i++) {
		buf[5+i] = gs_frame->data[i];
	}
	res += can_len;

	uint32_t can_id = gs_frame->can_id;
	if(can_id & CAN_RTR_FLAG) {
		can_len |= MCP_TXB_RTR_M;
	}

	buf[4] = can_len;

	uint8_t ext_flg = 0;
	if(can_id & CAN_EFF_FLAG) {
		ext_flg = 1;
		can_id &= CAN_EFF_MASK;
	} else {
		can_id &= CAN_SFF_MASK;
	}

	uint16_t ci = (uint16_t)(can_id & 0xFFFF);
	if(ext_flg) {
		buf[3] = (uint8_t) (ci & 0xFF);
		buf[2] = (uint8_t) (ci >> 8);
		ci = (uint16_t)(can_id >> 16);
		buf[1] = (uint8_t) (ci & 0x03);
		buf[1] += (uint8_t) ((ci & 0x1C) << 3);
		buf[1] |= MCP_TXB_EXIDE_M;
		buf[0] = (uint8_t) (ci >> 5);
	} else {
		buf[0] = (uint8_t) (ci >> 3);
		buf[1] = (uint8_t) ((ci & 0x07) << 5);
		buf[3] = 0;
		buf[2] = 0;
	}
	return res;
}

void mcp_to_err_host_frame(uint8_t mcp_err_flags, volatile gs_host_frame *gs_frame) {
	gs_frame->can_dlc = 8;
	gs_frame->flags = 0;
	for(uint8_t i=0; i<8; i++) {
		gs_frame->data[i] = 0;
	}
	gs_frame->can_id = CAN_ERR_FLAG;
	if(mcp_err_flags & MCP_EFLG_TXBO) {
		gs_frame->can_id |= CAN_ERR_BUSOFF;
	} else {
		gs_frame->can_id |= CAN_ERR_CTRL;
		if(mcp_err_flags & (MCP_EFLG_RX0OVR | MCP_EFLG_RX1OVR)) {
			gs_frame->flags = GS_CAN_FLAG_OVERFLOW;
			gs_frame->data[1] = CAN_ERR_CRTL_RX_OVERFLOW;
		}
		if(mcp_err_flags & MCP_EFLG_TXEP) {
			gs_frame->data[1] |= CAN_ERR_CRTL_TX_PASSIVE;
		}
		if(mcp_err_flags & MCP_EFLG_RXEP) {
			gs_frame->data[1] |= CAN_ERR_CRTL_RX_PASSIVE;
		}
		if(mcp_err_flags & MCP_EFLG_TXWAR) {
			gs_frame->data[1] |= CAN_ERR_CRTL_TX_WARNING;
		}
		if(mcp_err_flags & MCP_EFLG_RXWAR) {
			gs_frame->data[1] |= CAN_ERR_CRTL_RX_WARNING;
		}
	}
}
