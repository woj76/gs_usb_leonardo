# Licenses

The code in the gs_usb_leonardo project, see

        https://github.com/woj76/gs_usb_leonardo

is based on or inspired by the following projects / code / authors with the
following licenses. Note that hardly any of this code is copied directly without
modification or reused as a whole, there has been a lot of code rewriting,
removing, or extending. Some of the mentioned code has been only used for
reference, functionality understanding, constants, etc. The "licenses" folder
of this project contains the relevant license files.

1. The Arduino AVR core and extension libraries,
   see https://github.com/arduino/ArduinoCore-avr. In particular:

 - SPI.cpp with headers copyrighted and/or authored by Cristian Maglie
   <c.maglie@arduino.cc>, Paul Stoffregen <paul@pjrc.com>, Matthijs Kooijman
   <matthijs@stdin.nl>, Andrew J. Kroll <xxxajk@gmail.com>. This particular file
   has "either the GNU General Public License version 2 or the GNU Lesser General
   Public License version 2.1" stated.

 - USBCore.cpp with headers copyrighted and/or authored by Peter Barrett and
   Michael Dreher.

2. The Seeed Technology Inc. CAN-BUS Shield library for Arduino, see
   https://github.com/reeedstudio/CAN_BUS_Shield. In particular:

 - mcp_can.cpp with headers copyrighted and/or authored by
   Loovee (loovee@seeed.cc), Cory J. Fowler, Latonita, Woodward1, Mehtajaghvi,
   BykeBlast, TheRo0T, Tsipizic, ralfEdmund, Nathancheek, BlueAndi, Adlerweb,
   Btetz, Hurvajs, xboxpro1. This particular file states the "MIT License".

3. The Microchip 251x CAN Controller Linux kernel driver, see
   https://github.com/torvalds/linux. In particular:

 - mcp251x.c copyrighted and/or authored by Christian Pellegrin
   <chripell@evolware.org>, Raymarine UK, Ltd., Chris Elston, Katalix
   Systems, Ltd. Transitively the file mentions several other names. This
   particular file states "GNU General Public License version 2".

4. Geschwister Schneider USB/CAN device Linux driver, see
   https://github.com/torvalds/linux. In particular:

 - gs_usb.c copyrighted and/or authored by Geschwister Schneider Technologie-,
   Entwicklungs- und Vertriebs UG (Haftungsbeschränkt) and Hubert Denkmair.
   This particular file states "GNU General Public License version 2".

5. Finally the candleLight firmware project by Hubert Denkmair that inspired
   gs_usb_leonardo to start with, see https://github.com/HubertD/candleLight_fw.
