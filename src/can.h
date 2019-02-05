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

#ifndef CAN_H
#define CAN_H

#define CAN_EFF_FLAG			0x80000000
#define CAN_RTR_FLAG			0x40000000
#define CAN_ERR_FLAG			0x20000000
#define CAN_SFF_MASK			0x000007FF
#define CAN_EFF_MASK			0x1FFFFFFF

#define CAN_ERR_TX_TIMEOUT		0x00000001
#define CAN_ERR_CTRL			0x00000004
#define CAN_ERR_ACK			0x00000020
#define CAN_ERR_BUSOFF			0x00000040
#define CAN_ERR_CRTL_RX_WARNING		0x04
#define CAN_ERR_CRTL_TX_WARNING		0x08
#define CAN_ERR_CRTL_RX_PASSIVE		0x10
#define CAN_ERR_CRTL_TX_PASSIVE		0x20

#define CAN_ERR_CRTL_RX_OVERFLOW	0x01
#define CAN_ERR_CRTL_TX_OVERFLOW	0x02

#endif
