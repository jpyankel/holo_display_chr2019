#
# Makefile for holo_display_chr2019
#
# Joseph Yankel (jpyankel)
#

# === DEFINITIONS ===

# executable name
EXE = led_holo

# c compiler (using avr-gcc)
CC = avr-gcc

# avr-gcc compilation flags
# mmcu: Let avr-gcc know we are using ATtiny85
# DF_CPU: CPU clock freq. For this project, it is 8 MHz
# O3: Optimize for execution speed (because we want to sleep longer)
# Wall: Warns about everything that needs warning about
# g: Embed debug info; this won't affect the .hex file
CFLAG = -mmcu=attiny85 -DF_CPU=8000000 -O3 -Wall -g

# avr-gcc linker flags
# mmcu: Let avr-gcc linker know we are using ATtiny85
# Wl: denotes the start of the comma-separated linker options list
# Map: We want to output a linker map file
# $(OBJ_DIR)/$(EXE).map: the location/name of the output linker map file
LFLAG = -mmcu=attiny85 -Wl,-Map,$(OBJ_DIR)/$(EXE).map

# Source file directory
SRC_DIR = src

# OBJ folder (also build folder)
OBJ_DIR = build

# list of source files
SRC = $(wildcard $(SRC_DIR)/*.c)

# list of obj files from source files
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# === TARGETS ===

.PHONY: default flash fuse program clean

default: build

# WS2812B external library
$(OBJ_DIR)/light_ws2812.o: external/src/light_ws2812.c external/include/light_ws2812.h
	$(CC) $(CFLAG) -c external/src/light_ws2812.c -o $(OBJ_DIR)/light_ws2812.o

# compiles c source files to obj files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	# if build folder hasn't been built yet, create it now
	mkdir -p $(OBJ_DIR)

	$(CC) $(CFLAG) -c $< -o $@

# writes the fuse bits for our configuration
fuse:
	# this should be done before flashing or programming
	avrdude -p attiny85 -P /dev/ttyACM0 -c avrisp -b 19200 -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

# builds hex file to flash to mcu
build: $(OBJ_DIR)/light_ws2812.o $(OBJ)
	rm -f $(OBJ_DIR)/$(EXE).hex

	# link obj files to executable
	$(CC) $(LFLAG) -o $(OBJ_DIR)/$(EXE).elf $^

	# print out program and data size
	avr-size -C $(OBJ_DIR)/$(EXE).elf

	# extract debug view
	avr-objdump -h -S $(OBJ_DIR)/$(EXE).elf > $(OBJ_DIR)/$(EXE).lst

	# convert executable to .hex format
	avr-objcopy -j .text -j .data -O ihex $(OBJ_DIR)/$(EXE).elf $(OBJ_DIR)/$(EXE).hex

# flashes to ATtiny85 using output of build (requires arduino-as-isp connected)
flash:
	avrdude -v -p attiny85 -P /dev/ttyACM0 -c avrisp -b 19200 -U flash:w:$(OBJ_DIR)/$(EXE).hex

# runs build and flash
program: build flash

# deletes build files
clean:
	rm -f $(OBJ_DIR)/*
