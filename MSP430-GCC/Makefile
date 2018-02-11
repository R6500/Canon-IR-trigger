
# IR Trigger Makefile

# Folders to include in the path
# M:\MCU\msp430_Luna\UnxUtils\usr\local\wbin
# M:\MCU\msp430_Luna\msp430-gcc\bin

NAME      = IR_Trigger
OBJECTS   = main.o
MCU       = msp430g2553

#Compiler
GCC_DIR =  ../../msp430-gcc
CC      =  msp430-elf-gcc

# It seems that the GCC compiler don't know that
# the selected MCU has no hardware multiplier
# so we need to indicate that in the compiler flags

# Development version
#CFLAGS = -I $(GCC_DIR)/include -mmcu=$(MCU) -mhwmult=none -O0 -g

# Release version
CFLAGS = -I $(GCC_DIR)/include -mmcu=$(MCU) -mhwmult=none -O2

#Linker
LFLAGS = -L $(GCC_DIR)/include -Wl,-Map=$(NAME).map

#Object Dump
#OD     = ${GCC_DIR}/bin/msp430-elf-objdump
OD     = msp430-elf-objdump
SZ     = msp430-elf-size

.PHONY: all clean 

all: ${NAME}.elf ${NAME}.lst

${NAME}.elf: ${OBJECTS}
	$(CC) $(CFLAGS) $(LFLAGS) $? -o $(NAME).elf

${NAME}.lst: ${NAME}.elf
	${OD} -dSt $^ > $@
	${SZ} ${NAME}.elf

clean:
	rm -f ${NAME}.elf ${NAME}.lst ${OBJECTS}

#project dependencies
main.o: main.c io430masks.h Makefile

