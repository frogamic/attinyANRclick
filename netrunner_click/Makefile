# File:   Makefile
# Author: Dominic Shelton
# Date:   3 Dec 2014
# Descr:  Makefile for AVR programs
#         Adapted from that supplied in ENCE260 written by M. Hayes.
#         Influenced by WinAVR sample written by E Weddington, J Wunsch, et al.


# The target to be built (no extension)
TARGET      = main

# List of C source files
SRC         = $(TARGET).c
SRC         += buttons.c jumpers.c

# Define all object files
OBJ         = $(SRC:.c=.o)

# C definitions
CFLAGS      = -std=gnu99 -Os -Wall -Wstrict-prototypes -Wextra
LDFLAGS     = -lm

# Programmer definitions
MMCU        = atmega8
FORMAT      = ihex
INTERFACE   = usbtiny
PART        = m8
CFLAGS      += -mmcu=$(MMCU)

# Programming Definitions
CC          = avr-gcc
OBJCOPY     = avr-objcopy
SIZE        = avr-size
DEL         = rm
PROGRAMMER  = avrdude

PROGRAMMER_FLAGS = -p $(PART) -c $(INTERFACE)
WRITE_FLASH = -U flash:w:$(TARGET).hex

# Default target.
all: $(TARGET).elf size


# Compile: create object files from C source files.
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@


# Link: create output file (executable) from object files.
$(TARGET).elf: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)


# Create hex file for programming from executable file.
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O $(FORMAT) $< $@


# Target: program project.
program: size $(TARGET).hex
	$(PROGRAMMER) $(PROGRAMMER_FLAGS) $(WRITE_FLASH)


# Get size of ELF file
size:
	@if [ -s $(TARGET).elf ]; then echo; $(SIZE) $(TARGET).elf; echo; fi


# Target: clean project.
clean: 
	@$(DEL) *.d *.o *.elf *.hex 2>/dev/null || true


# Automatically generate C source code dependencies. 
# (Code originally taken from the GNU make user manual and modified)
%.d: %.c
	@set -e; $(CC) -MM $(CFLAGS) $< \
	| sed 's,\(.*\)\.o[ :]*,\1.o \1.d : ,g' > $@; \
	[ -s $@ ] || rm -f $@


-include $(SRC:.c=.d)


.PHONY: program clean size
