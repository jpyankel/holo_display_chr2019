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

# avr-gcc flags
# mmcu: Let avr-gcc know we are using ATtiny85
# DF_CPU: CPU clock freq. For this project, it is 8 MHz
# O3: Optimize for execution speed (because we want to sleep longer)
# Wall: Warns about everything that needs warning about
# g: Embed debug info; this won't affect the .hex file
CFLAG = -mmcu=attiny85 -DF_CPU=8000000 -O3 -Wall -g

# Source file directory
SRC_DIR = src

# OBJ folder (also build folder)
OBJ_DIR = build

# list of source files
SRC = $(wildcard $(SRC_DIR)/*.c)

# list of obj files from source files
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# === TARGETS ===

.PHONY: default flash program clean

default: build

# compiles c source files to obj files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	# if build folder hasn't been built yet, create it now
	mkdir -p $(OBJ_DIR)

	$(CC) $(CFLAG) -c $< -o $@

# builds hex file to flash to mcu
build: $(OBJ)
	# link obj files to executable
	$(CC) -o $(OBJ_DIR)/$(EXE).elf $^

	# print out program and data size
	avr-size -C $(OBJ_DIR)/$(EXE).elf

	# extract debug view
	avr-objdump -h -S $(OBJ_DIR)/$(EXE).elf > $(OBJ_DIR)/$(EXE).lst

	# convert executable to .hex format
	avr-objcopy -j .text -j .data -O ihex $(OBJ_DIR)/$(EXE).elf $(OBJ_DIR)/$(EXE).hex

# flashes to ATtiny85 using output of build (requires arduino-as-isp connected)
flash:
	avrdude -p attiny85 -P /dev/ttyACM0 -c avrisp -b 19200 -U flash:w:$(OBJ_DIR)/$(EXE).hex

# runs build and flash
program: build flash

# deletes build files
clean:
	rm -f $(OBJ_DIR)/*
