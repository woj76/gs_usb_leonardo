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

/* The interface to the MPC2515 SPI chip */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "spi.h"
#include "mcp.h"
#include "bool.h"

uint8_t mcp_device_mode = MODE_NORMAL;
uint8_t mcp_buf_in[2][13];
uint8_t mcp_buf_out[13];
uint8_t mcp_err_flags;
uint8_t mcp_cnfs[3];

#define mcp_select()	(PORTB &= 0xFE)
#define mcp_unselect()	(PORTB |= 0x01)

void mcp_reset_spi() {
	register uint8_t _sreg = SREG;
	cli();
	mcp_select();
	spi_transfer8(MCP_RESET);
	mcp_unselect();
	SREG = _sreg;
}

uint8_t mcp_read_register_spi(const uint8_t address) {
	register uint8_t _sreg = SREG;
	cli();
	mcp_select();
	spi_transfer8(MCP_READ);
	spi_transfer8(address);
	uint8_t ret = spi_transfer8(0);
	mcp_unselect();
	SREG = _sreg;
	return ret;
}

void mcp_read_registers_spi(const uint8_t address, uint8_t* values, const uint8_t n) {
	register uint8_t _sreg = SREG;
	cli();
	mcp_select();
	spi_transfer8(MCP_READ);
	spi_transfer8(address);
	spi_transfer(values, n);
	mcp_unselect();
	SREG = _sreg;
}

void mcp_set_register_spi(const uint8_t address, const uint8_t value) {
	register uint8_t _sreg = SREG;
	cli();
	mcp_select();
	spi_transfer8(MCP_WRITE);
	spi_transfer8(address);
	spi_transfer8(value);
	mcp_unselect();
	SREG = _sreg;
}

void mcp_set_registers_spi(const uint8_t address, uint8_t* values, const uint8_t n) {
	register uint8_t _sreg = SREG;
	cli();
	mcp_select();
	spi_transfer8(MCP_WRITE);
	spi_transfer8(address);
	spi_transfer(values, n);
	mcp_unselect();
	SREG = _sreg;
}

void mcp_modify_register_spi(const uint8_t address, const uint8_t mask, const uint8_t data) {
	register uint8_t _sreg = SREG;
	cli();
	mcp_select();
	spi_transfer8(MCP_BITMOD);
	spi_transfer8(address);
	spi_transfer8(mask);
	spi_transfer8(data);
	mcp_unselect();
	SREG = _sreg;
}

uint8_t mcp_read_status_spi() {
	register uint8_t _sreg = SREG;
	cli();
	mcp_select();
	spi_transfer8(MCP_READ_STATUS);
	uint8_t ret = spi_transfer8(0);
	mcp_unselect();
	SREG = _sreg;
	return ret;
}

uint8_t mcp_mode_one_shot(uint8_t one_shot) {
	uint8_t shot_mode = one_shot ? MODE_ONESHOT : ~MODE_ONESHOT;
	mcp_modify_register_spi(MCP_CANCTRL, MODE_ONESHOT, shot_mode);
	if((mcp_read_register_spi(MCP_CANCTRL) & MODE_ONESHOT) == (one_shot ? shot_mode : 0)) {
		return OK;
	}
	return FAIL;
}

uint8_t mcp_set_ctrl_mode(const uint8_t mode) {
	mcp_modify_register_spi(MCP_CANCTRL, MODE_MASK, mode);
	if((mcp_read_register_spi(MCP_CANCTRL) & MODE_MASK) == mode) {
		return OK;
	}
	return FAIL;
}

uint8_t mcp_config_rate() {
	mcp_set_register_spi(MCP_CNF1, mcp_cnfs[0]);
	mcp_set_register_spi(MCP_CNF2, mcp_cnfs[1]);
	mcp_set_register_spi(MCP_CNF3, mcp_cnfs[2]);
	return OK;
}

void mcp_init_buffers() {
	uint8_t a1 = MCP_TXB0CTRL;
	uint8_t a2 = MCP_TXB1CTRL;
	uint8_t a3 = MCP_TXB2CTRL;
	for(uint8_t i=0; i < 14; i++) {
		mcp_set_register_spi(a1++, 0);
		mcp_set_register_spi(a2++, 0);
		mcp_set_register_spi(a3++, 0);
	}
	mcp_set_register_spi(MCP_RXB0CTRL, 0);
	mcp_set_register_spi(MCP_RXB1CTRL, 0);
}

uint8_t mcp_init() {
	mcp_reset_spi();
	uint8_t res = mcp_set_ctrl_mode(MODE_CONFIG);
	if(res) {
		return res;
	}
	res = mcp_config_rate();
	if(res) {
		return res;
	}
	mcp_init_buffers();
	// See the note in main.c - interrupt based reception does not work well in loopback mode
	if(mcp_device_mode == MODE_LOOPBACK) {
		mcp_set_register_spi(MCP_CANINTE, MCP_NO_INT);
	} else {
		mcp_set_register_spi(MCP_CANINTE, MCP_RX0IF | MCP_RX1IF | MCP_TX0IF | MCP_TX1IF | MCP_TX2IF | MCP_ERRIF);
	}
	mcp_modify_register_spi(MCP_RXB0CTRL, MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK, MCP_RXB_RX_STDEXT | MCP_RXB_BUKT_MASK);
	mcp_modify_register_spi(MCP_RXB1CTRL, MCP_RXB_RX_MASK, MCP_RXB_RX_STDEXT);
	return mcp_set_ctrl_mode(mcp_device_mode);
}

void mcp_set_mode_normal() {
	mcp_device_mode = MODE_NORMAL;
}

void mcp_set_mode_loopback() {
	mcp_device_mode = MODE_LOOPBACK;
}

void mcp_set_mode_listen() {
	mcp_device_mode = MODE_LISTENONLY;
}

uint8_t mcp_begin() {
	DDRB |= 0x01;
	mcp_unselect();
	spi_init();
	return mcp_init();
}

inline void mcp_enqueue_can_frame(uint8_t txbctrl_index, uint8_t len) {
	uint8_t txctrl = MCP_TXBCTRL(txbctrl_index); 
	mcp_set_registers_spi(txctrl+1, mcp_buf_out, len);
	uint8_t t_idx = txbctrl_index + 1;
	if(t_idx == MCP_N_TXBUFFERS) {
		t_idx = 0;
	}
	mcp_modify_register_spi(MCP_TXBCTRL(t_idx), MCP_TXB_TXP10_M, 2);			// priority 2
	t_idx++;
	if(t_idx == MCP_N_TXBUFFERS) {
		t_idx = 0;
	}
	mcp_modify_register_spi(MCP_TXBCTRL(t_idx), MCP_TXB_TXP10_M, 1);			// priority 1
	mcp_modify_register_spi(txctrl, MCP_TXB_TXREQ_M | MCP_TXB_TXP10_M, MCP_TXB_TXREQ_M);	// priority 0
}

inline uint8_t mcp_send_can_frame(uint8_t txbctrl_index) {
	uint16_t time_out = MCP_SEND_TIMEOUT;
	uint8_t tx_int_mask = (MCP_TX0IF << txbctrl_index);
	uint8_t res = 0;
	while(!res && --time_out) {
		res = (mcp_read_register_spi(MCP_CANINTF) & tx_int_mask);
	}
	if(res) {
		mcp_modify_register_spi(MCP_CANINTF, tx_int_mask, 0);
		return OK;
	}
	return FAIL;
}

uint8_t mcp_service_interrupt() {
	uint8_t canintf_eflag[2];
	mcp_read_registers_spi(MCP_CANINTF, canintf_eflag, 2);
	uint8_t res = canintf_eflag[0];
	if(res & MCP_RX0IF) {
		mcp_read_registers_spi(MCP_RXBUF_0, mcp_buf_in[0], 13);
		mcp_modify_register_spi(MCP_CANINTF, MCP_RX0IF, 0);
	} 
	if(res & MCP_RX1IF) {
		mcp_read_registers_spi(MCP_RXBUF_1, mcp_buf_in[1], 13);
		mcp_modify_register_spi(MCP_CANINTF, MCP_RX1IF, 0);
	}
	if(res & MCP_TX0IF) {
		mcp_modify_register_spi(MCP_CANINTF, MCP_TX0IF, 0);
	}
	if(res & MCP_TX1IF) {
		mcp_modify_register_spi(MCP_CANINTF, MCP_TX1IF, 0);
	}
	if(res & MCP_TX2IF) {
		mcp_modify_register_spi(MCP_CANINTF, MCP_TX2IF, 0);
	}
	if(res & MCP_ERRIF) {
		mcp_err_flags = canintf_eflag[1];
		mcp_modify_register_spi(MCP_CANINTF, MCP_ERRIF, 0);
	}
	return res;
}

uint8_t mcp_receive_can_frame() {
	uint8_t stat = mcp_read_status_spi();
	if(stat & MCP_STAT_RX0IF) {
		mcp_read_registers_spi(MCP_RXBUF_0, mcp_buf_in[0], 13);
		mcp_modify_register_spi(MCP_CANINTF, MCP_RX0IF, 0);
		return 1;
	}else if(stat & MCP_STAT_RX1IF) {
		mcp_read_registers_spi(MCP_RXBUF_1, mcp_buf_in[1], 13);
		mcp_modify_register_spi(MCP_CANINTF, MCP_RX1IF, 0);
		return 2;
	}
	return 0;
}

/* These are currently unused, and the gs_usb driver for Linux itself
   does not have the HW filter capability. */

void mcp_write_id(const uint8_t mcp_addr, const uint8_t ext, const uint32_t id) {
	uint16_t canid = (uint16_t)(id & 0xFFFF);
	uint8_t buf[4];

	if(ext) {
		buf[MCP_EID0] = (uint8_t) (canid & 0xFF);
		buf[MCP_EID8] = (uint8_t) (canid >> 8);
		canid = (uint16_t)(id >> 16);
		buf[MCP_SIDL] = (uint8_t) (canid & 0x03);
		buf[MCP_SIDL] += (uint8_t) ((canid & 0x1C) << 3);
		buf[MCP_SIDL] |= MCP_TXB_EXIDE_M;
		buf[MCP_SIDH] = (uint8_t) (canid >> 5);
	} else {
		buf[MCP_SIDH] = (uint8_t) (canid >> 3);
		buf[MCP_SIDL] = (uint8_t) ((canid & 0x07) << 5);
		buf[MCP_EID0] = 0;
		buf[MCP_EID8] = 0;
	}
	mcp_set_registers_spi(mcp_addr, buf, 4);
}

uint8_t mcp_init_mask(uint8_t num, uint8_t ext, uint32_t data) {
	uint8_t res = mcp_set_ctrl_mode(MODE_CONFIG);
	if(res) {
		return res;
	}
	if(num == 0) {
		mcp_write_id(MCP_RXM0SIDH, ext, data);
	} else if(num == 1) {
		mcp_write_id(MCP_RXM1SIDH, ext, data);
	}
	return mcp_set_ctrl_mode(mcp_device_mode);
}

uint8_t mcp_init_filt(uint8_t num, uint8_t ext, uint32_t data) {
	uint8_t res = mcp_set_ctrl_mode(MODE_CONFIG);
	if(res) {
		return res;
	}
	switch(num) {
		case 0: mcp_write_id(MCP_RXF0SIDH, ext, data); break;
		case 1: mcp_write_id(MCP_RXF1SIDH, ext, data); break;
		case 2: mcp_write_id(MCP_RXF2SIDH, ext, data); break;
		case 3: mcp_write_id(MCP_RXF3SIDH, ext, data); break;
		case 4: mcp_write_id(MCP_RXF4SIDH, ext, data); break;
		case 5: mcp_write_id(MCP_RXF5SIDH, ext, data); break;
		default: return FAIL;
	}
	return mcp_set_ctrl_mode(mcp_device_mode);
}
