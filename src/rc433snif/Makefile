#
# Copyright(C) 2021 Robinson (Bob) Mittman. All Rights Reserved.
# Licensed under the MIT license. 
# See LICENSE file in the project root for details.
#

PROG = rc433snif

MCU = atmega328p

F_CPU = 8000000UL
#F_CPU = 16000000UL

CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
CFLAGS = -std=c99 -Wall -Os -DF_CPU=${F_CPU} -I. -I../include
OPTIONS = -mmcu=${MCU} -g

PORT = ft0

CFILES = rc433snif.c io.c rc433rx_uart.c

all: elf hex lst

hex: ${PROG}.hex
elf: ${PROG}.elf
lst: ${PROG}.lst

${PROG}.elf: ${CFILES}
	${CC} ${CFLAGS} ${OPTIONS} -o $@ $^

${PROG}.hex: ${PROG}.elf
	${OBJCOPY} -j .text -j .data -O ihex $< $@

${PROG}.lst: ${PROG}.elf
	${OBJDUMP} -S $< > $@


flash: ${PROG}.hex
	avrdude -p ${MCU} -c ttl232r -U flash:w:$<:i -F -P ${PORT}

clean:
	rm -f *.o *.elf *.lst *.hex
