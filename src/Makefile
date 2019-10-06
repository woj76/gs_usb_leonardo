# Makefile for the gs_usb_leonardo project
#
# This requires the AVR tools and libs on your Linux installation.
# 
# To produce the AVR elf binary say "make" or "make elf".
# To produce the Leonardo uploadable file say "make hex".
# To install it onto the Leonardo-CANBUS board say "make install", alternatively
# "make ACM_PORT=/dev/ttyACM<n> install" if your board is not connected as /dev/ttyACM0.
# See README.md for further details.

ifndef ACM_PORT
ACM_PORT = /dev/ttyACM0
endif

# Typically a lot of AVR code needs the F_CPU clock constant set, not this project though,
# may there be need for this, use -DF_CPU=16000000L in CFLAGS

CFLAGS = -mmcu=atmega32u4 -Os -ffunction-sections -fdata-sections -flto
LDFLAGS = -mmcu=atmega32u4 -fuse-linker-plugin -Wl,--gc-sections
OBJ_FILES = spi.o mcp.o usb.o gs_usb.o mcp_gs.o main.o
ELF_FILE = gs_usb_leonardo.elf
HEX_FILE = gs_usb_leonardo.hex

default: elf

elf: $(ELF_FILE)
hex: $(HEX_FILE)

%.o: %.c
	@echo -n "Compiling $<... "
	@avr-gcc -c $(CFLAGS) $<
	@echo "OK."

$(ELF_FILE): $(OBJ_FILES)
	@echo -n "Linking $(ELF_FILE)... "
	@avr-gcc $(LDFLAGS) $(OBJ_FILES) -o $(ELF_FILE)
	@echo "OK."

$(HEX_FILE): $(ELF_FILE)
	@echo -n "Generating flash file $(HEX_FILE)... "
	@avr-objcopy -O ihex -R .eeprom $(ELF_FILE) $(HEX_FILE)
	@echo "OK."

install: $(HEX_FILE)
	@echo -n "Uploading / waiting for port $(ACM_PORT) (device boot reset)"
	@while test ! -e $(ACM_PORT); do echo -n "."; sleep 1; done
	@echo " found.";
	@avrdude -patmega32u4 -cavr109 -P$(ACM_PORT) -b57600 -D -Uflash:w:$(HEX_FILE):i

clean:
	@echo -n "Removing binary files... "
	@rm -f $(OBJ_FILES) $(ELF_FILE) $(HEX_FILE)
	@echo "OK."
