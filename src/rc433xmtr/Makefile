#
# Copyright(C) 2021 Robinson (Bob) Mittman. All Rights Reserved.
# Licensed under the MIT license. 
# See LICENSE file in the project root for details.
#

PROG = rc433xmtr

MCU = atmega328p
F_CPU = 16000000UL
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
CFLAGS = -std=c99 -Wall -Ofast -DF_CPU=${F_CPU} -I. -I ../include
OPTIONS = -mmcu=${MCU} -g 
FTPORT = ft1

CFILES = io.c rc433xmtr.c rc433tx_uart.c

all: elf hex lst

hex: ${PROG}.hex
elf: ${PROG}.elf
lst: ${PROG}.lst

${PROG}.elf: ${CFILES}
	${CC} ${CFLAGS} ${OPTIONS} -o $@ $^

${PROG}.hex: ${PROG}.elf
	${OBJCOPY} -j .text -j .data -O ihex $< $@

${PROG}.lst: ${PROG}.elf
	${OBJDUMP} -D -S $< > $@


flash: ${PROG}.hex
	avrdude -p ${MCU} -c ttl232r -U flash:w:$<:i -F -P ${FTPORT}

clean:
	rm -f *.o *.elf *.lst *.hex

