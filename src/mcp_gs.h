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

#ifndef MCP_GS_H
#define MCP_GS_H

#include <stdint.h>
#include "gs_usb.h"

void gs_bittiming_to_mcp(volatile gs_device_bittiming* bittiming, uint8_t triple_sample, uint8_t* cnfs);
void mcp_to_gs_host_frame(uint8_t* buf, volatile gs_host_frame* gs_frame);
uint8_t gs_host_frame_to_mcp(volatile gs_host_frame* gs_frame, uint8_t* buf);
void mcp_to_err_host_frame(uint8_t mcp_err_flags, volatile gs_host_frame *gs_frame);

#endif
