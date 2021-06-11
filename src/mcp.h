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

#ifndef MCP_H
#define MCP_H

void mcp_set_mode_normal();
void mcp_set_mode_loopback();
void mcp_set_mode_listen();
uint8_t mcp_mode_one_shot(uint8_t one_shot);

uint8_t mcp_begin(uint8_t use_rb2);
uint8_t mcp_init_mask(uint8_t num, uint8_t ext, uint32_t data);
uint8_t mcp_init_filt(uint8_t num, uint8_t ext, uint32_t data);

extern uint8_t mcp_cnfs[];
extern uint8_t mcp_buf_in[][13];
extern uint8_t mcp_buf_out[];
extern uint8_t mcp_err_flags;

void mcp_enqueue_can_frame(uint8_t txbctrl_index, uint8_t len);
uint8_t mcp_send_can_frame(uint8_t txbctrl_index);
uint8_t mcp_receive_can_frame();
uint8_t mcp_service_interrupt();

#define MCP_SIDH		0
#define MCP_SIDL		1
#define MCP_EID8		2
#define MCP_EID0		3

#define MCP_TXB_EXIDE_M		0x08
#define MCP_DLC_MASK		0x0F
//#define MCP_RTR_MASK		0x40

#define MCP_RXB_RX_ANY		0x60
#define MCP_RXB_RX_EXT		0x40
#define MCP_RXB_RX_STD		0x20
#define MCP_RXB_RX_STDEXT	0x00
#define MCP_RXB_RX_MASK		0x60
#define MCP_RXB_BUKT_MASK	0x04

// TXBnCTRL

#define MCP_TXB_TXBUFE_M	0x80
#define MCP_TXB_ABTF_M		0x40
#define MCP_TXB_MLOA_M		0x20
#define MCP_TXB_TXERR_M		0x10
#define MCP_TXB_TXREQ_M		0x08
#define MCP_TXB_TXIE_M		0x04
#define MCP_TXB_TXP10_M		0x03

#define MCP_TXB_RTR_M		0x40
//#define MCP_RXB_IDE_M		0x08
#define MCP_RXB_RTR_M		0x40

#define MCP_STAT_RXIF_MASK	0x03
#define MCP_STAT_RX0IF		0x01
#define MCP_STAT_RX1IF		0x02

#define MCP_EFLG_RX1OVR		0x80
#define MCP_EFLG_RX0OVR		0x40
#define MCP_EFLG_TXBO		0x20
#define MCP_EFLG_TXEP		0x10
#define MCP_EFLG_RXEP		0x08
#define MCP_EFLG_TXWAR		0x04
#define MCP_EFLG_RXWAR		0x02
#define MCP_EFLG_EWARN		0x01
#define MCP_EFLG_ERRORMASK	0xF8

// MCP2515 registers

#define MCP_RXF0SIDH		0x00
#define MCP_RXF0SIDL		0x01
#define MCP_RXF0EID8		0x02
#define MCP_RXF0EID0		0x03
#define MCP_RXF1SIDH		0x04
#define MCP_RXF1SIDL		0x05
#define MCP_RXF1EID8		0x06
#define MCP_RXF1EID0		0x07
#define MCP_RXF2SIDH		0x08
#define MCP_RXF2SIDL		0x09
#define MCP_RXF2EID8		0x0A
#define MCP_RXF2EID0		0x0B
#define MCP_CANSTAT		0x0E
#define MCP_CANCTRL		0x0F
#define MCP_RXF3SIDH		0x10
#define MCP_RXF3SIDL		0x11
#define MCP_RXF3EID8		0x12
#define MCP_RXF3EID0		0x13
#define MCP_RXF4SIDH		0x14
#define MCP_RXF4SIDL		0x15
#define MCP_RXF4EID8		0x16
#define MCP_RXF4EID0		0x17
#define MCP_RXF5SIDH		0x18
#define MCP_RXF5SIDL		0x19
#define MCP_RXF5EID8		0x1A
#define MCP_RXF5EID0		0x1B
#define MCP_TEC			0x1C
#define MCP_REC			0x1D
#define MCP_RXM0SIDH		0x20
#define MCP_RXM0SIDL		0x21
#define MCP_RXM0EID8		0x22
#define MCP_RXM0EID0		0x23
#define MCP_RXM1SIDH		0x24
#define MCP_RXM1SIDL		0x25
#define MCP_RXM1EID8		0x26
#define MCP_RXM1EID0		0x27
#define MCP_CNF3		0x28
#define MCP_CNF2		0x29
#define MCP_CNF1		0x2A
#define MCP_CANINTE		0x2B
#define MCP_CANINTF		0x2C
#define MCP_EFLG		0x2D
#define MCP_TXB0CTRL		0x30
#define MCP_TXB1CTRL		0x40
#define MCP_TXB2CTRL		0x50
#define MCP_TXBCTRL(i)		(MCP_TXB0CTRL + (i << 4))
#define MCP_RXB0CTRL		0x60
#define MCP_RXB0SIDH		0x61
#define MCP_RXB1CTRL		0x70
#define MCP_RXB1SIDH		0x71
#define MCP_TX_INT		0x1C
#define MCP_TX01_INT		0x0C
#define MCP_RX_INT		0x03
#define MCP_NO_INT		0x00

#define MCP_TX01_MASK		0x14
#define MCP_TX_MASK		0x54

// SPI instructions

#define MCP_WRITE		0x02
#define MCP_READ		0x03
#define MCP_BITMOD		0x05
#define MCP_LOAD_TX0		0x40
#define MCP_LOAD_TX1		0x42
#define MCP_LOAD_TX2		0x44

#define MCP_RTS_TX0		0x81
#define MCP_RTS_TX1		0x82
#define MCP_RTS_TX2		0x84
#define MCP_RTS_ALL		0x87
#define MCP_READ_RX0		0x90
#define MCP_READ_RX1		0x94
#define MCP_READ_STATUS		0xA0
#define MCP_RX_STATUS		0xB0
#define MCP_RESET		0xC0

// CANCTRL

#define MODE_NORMAL		0x00
#define MODE_SLEEP		0x20
#define MODE_LOOPBACK		0x40
#define MODE_LISTENONLY		0x60
#define MODE_CONFIG		0x80
#define MODE_POWERUP		0xE0
#define MODE_MASK		0xE0
#define ABORT_TX		0x10
#define MODE_ONESHOT		0x08

// CNF1

#define SJW1			0x00
#define SJW2			0x40
#define SJW3			0x80
#define SJW4			0xC0
#define CNF1_SJW_SHIFT		6

// CNF2

#define BTLMODE			0x80
#define SAMPLE_1X		0x00
#define SAMPLE_3X		0x40
#define CNF2_PS1_SHIFT		3

// CNF3

#define SOF_ENABLE		0x80
#define WAKFIL_ENABLE		0x40

// CANINTF

#define MCP_RX0IF		0x01
#define MCP_RX1IF		0x02
#define MCP_TX0IF		0x04
#define MCP_TX1IF		0x08
#define MCP_TX2IF		0x10
#define MCP_ERRIF		0x20
#define MCP_WAKIF		0x40
#define MCP_MERRF		0x80

#define MCP_N_TXBUFFERS		3
#define MCP_RXBUF_0		MCP_RXB0SIDH
#define MCP_RXBUF_1		MCP_RXB1SIDH
#define MCP_SEND_TIMEOUT	500

#endif
