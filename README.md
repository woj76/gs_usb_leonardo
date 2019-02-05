# gs_usb_leonardo

OVERVIEW

This is the gs_usb_leonardo project that adds native Linux SocketCAN capability
to the Arduino Leonardo (Atmega 32U4) / MCP2515 based USB device from
HobbyTronics, see here:

        http://www.hobbytronics.co.uk/leonardo-canbus

Following the style and idea from the candleLight_fw project, the gs_usb_leonardo
firmware pretends to be a Geschwister Schneider device that is supported on newer
Linux kernels through the gs_usb kernel module.

The C code in this project is based on a handful of Arduino libraries, but in
itself is neither library based (it is modularised though), nor Arduino, nor C++.
Just a clean, standalone C code for AVR that provides the native USB interface to
the MPC2515 CAN-BUS controller. The main design goal of the code is to be as fast
and efficient as possible. During testing on my laptop with my private Bosch ECU
flasher it turned out that saved clock cycles in the Leonardo code can make a
difference between random device lock-down and rock stability (especially when
"flooding" the ECU with continuous FC CAN frames when transferring flash data in
2KB chained blocks). To achieve the required performance, the code uses all three
MCP2515 transmit buffers (which BTW, the Linux mcp251x driver does not do, not
sure why) and relies heavily on the MCP transmit and receive interrupts.

REQUIREMENTS

This code is developed for the mentioned board from HobbyTronics (and only that
board, pins and ports used on that board are currently hard coded in), so
obviously you need the board, you need to assemble it / solder it up, and connect
it to your CAN system and PC. To compile and install (that you do on your Linux)
the firmware, you need to get the AVR compiler and tools installed. On Ubuntu
18.04 these are the packages you need:

       make, avr-libc, binutils-avr, gcc-avr, avrdude

with possible dependencies, your mileage may vary depending on your Linux distro.
It might be also possible to compile the code on Windows, but (a) I wouldn't know
how to go about it (though Arduino IDE ships with all the required tools I guess),
(b) the firmware is only for the Linux SocketCAN and gs_usb driver so far, so
what would be the point anyway. Otherwise, it is a good idea to have canutils
(candump, cangen, etc.) also installed.

INSTALLING

Go to the src directory of the project and say "make" to check that the code
compiles cleanly. If so, you can go ahead to installing it on your Leonardo
CAN-BUS board. Connect the board with the USB cable to your PC and...

FIRST READ THIS WARNING MESSAGE -- This firmware is _not_ an Arduino IDE
compatible code, it fully replaces your typical Arduino sketch and core (not the
bootloader, fortunately). This means that to upload it, you need to reset the
device before the uploading process (avrdude) to get into the bootloader. You
also need to do the same when attempting to upload Arduino IDE compatible code
back to the device. That is, the standard attempt of the AVR flash utility
avrdude to get into the device by manipulating the ACM serial port will not work,
simply because after gs_usb_leonardo is installed the ACM port disappears and is
only present when in the bootloader mode. The file reset_ground_pads.jpg shows
the ground and reset pads on the board that you need to short to reset the device
into the bootloader -- END OF WARNING.

So now you can say "make install", the upload hex file will be generated and the
installer will wait for the indicated ACM port (the default is /dev/ttyACM0, to
change it use "make ACM_PORT=/dev/ttyACMX install" instead) to appear. When
installing the code on a board that currently has Arduino IDE compatible sketch
this should just go through and you should end up with the firmware installed.
When updating the existing gs_usb_leonardo code now is the time to reset the
board by momentarily shorting the ground and reset pads on the board.

NOTE: I have notorious problems being able to do this on a freshly rebooted
system, I typically need two attempts / board reconnected to get this going.

The ready to upload hex file (gs_usb_leonardo.hex) is distributed in the root
directory of the project for your convenience.

USING

You should be all set, just to make sure reconnect the Leonardo CAN-BUS board to
your PC. Your Linux should recognise the device as gs_usb compatible, you should
be able to see the gs_usb module loaded when saying "lsmod" and some gs_usb
messages when saying "dmesg" stating a successful driver load.

What remains is to bring up the CAN interface. You can do this manually by saying
(sudo will ask you for your user password):

        sudo ip link set can0 type can bitrate 500000 restart-ms 10
        sudo ip link set can0 up

where 500000 is the CAN bus bitrate you want. Check with "sudo ip link show" to
see that the can0 interface was activated.

If you want this can0 interface setup to come up automatically on connecting the
device and be more permanent you have to modify your system network settings. On
my Ubuntu 18.04 I added this (you need sudo for this) to /etc/network/interfaces:

        allow-hotplug can0
        iface can0 can static
            bitrate 500000
            up /sbin/ip link set $IFACE down
            up /sbin/ip link set $IFACE type can bitrate 500000 restart-ms 10
            up /sbin/ip link set $IFACE up

You can also configure the interface to work in loopback mode, this is useful to
check your installation without any real CAN traffic or device. Just add
"loopback on" before "restart-ms 10" in either of the case above.

Finally, I did not find it necessary to play with txqueuelen parameter for the
can0 interface that one sees often quoted on the Internet in the context of
troubleshooting lost CAN messages.

You are ready! You can watch your CAN traffic by saying "candump can0" or
generate some (for example, when in loopback mode) with "cangen can0". Or use any
other SocketCAN based software.

LEDS

The code is configured for a specific set of LEDS that I have connected on my
private HobbyTronics board to indicate the power status and in/out traffic.
These are obviously not necessary, if you do not have them nothing bad will
happen, you will just miss the light show ;), no need to modify any code. If you
do have them, but connected differently, look into the leds.h file in the src
directory to see what you need to change.

DATASHEETS

Links to some chip documentations for the curious ones:

 - Atmega 32U4: http://www.mouser.com/ds/2/268/7766s-1065155.pdf
 - MCP2515: http://www.hobbytronics.co.uk/datasheets/MCP2515.pdf

AUTHOR / LICENSE / DISCLAIMER

The author of this project and code is Wojciech Mostowski
<wojciech.mostowski@gmail.com>. The code is distributed under the terms of
GNU General Public License. The "licenses" directory contains a copy of the
version 2 of the license. The author of this project takes no responsibility nor
is liable for any damage or loss caused by the use of the project code.
Furthermore, no suitability for any particular purpose is promised. USE AT YOUR
OWN RISK!!!. Otherwise see the GPL license conditions for details on the lack of
any warranty. Having said that, you can always drop the author a line if/when
you find the project useful.
