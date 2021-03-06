/*
 * Copyright(C) 2021 Robinson (Bob) Mittman. All Rights Reserved.
 * Licensed under the MIT license. 
 * See LICENSE file in the project root for details.
 *
 */

#include "io.h"
#include <util/delay.h>
#include <avr/interrupt.h> 
#include <avr/sleep.h> 

#define USART_BAUDRATE 4800

#define ASYNCHRONOUS (0 << UMSEL00)

#define DISABLED    (0 << UPM00)
#define EVEN_PARITY (2 << UPM00)
#define ODD_PARITY  (3 << UPM00)
#define PARITY_MODE DISABLED

#define ONE_BIT     (0 << USBS0)
#define TWO_BIT     (1 << USBS0)
#define STOP_BIT    ONE_BIT

#define FIVE_BIT    (0 << UCSZ00)
#define SIX_BIT     (1 << UCSZ00)
#define SEVEN_BIT   (2 << UCSZ00)
#define EIGHT_BIT   (3 << UCSZ00)
#define DATA_BIT   EIGHT_BIT 

const uint8_t crc5lut[256] = {
	0x00, 0x0e, 0x1c, 0x12, 0x11, 0x1f, 0x0d, 0x03, 
	0x0b, 0x05, 0x17, 0x19, 0x1a, 0x14, 0x06, 0x08, 
	0x16, 0x18, 0x0a, 0x04, 0x07, 0x09, 0x1b, 0x15, 
	0x1d, 0x13, 0x01, 0x0f, 0x0c, 0x02, 0x10, 0x1e, 
	0x05, 0x0b, 0x19, 0x17, 0x14, 0x1a, 0x08, 0x06, 
	0x0e, 0x00, 0x12, 0x1c, 0x1f, 0x11, 0x03, 0x0d,
	0x13, 0x1d, 0x0f, 0x01, 0x02, 0x0c, 0x1e, 0x10, 
	0x18, 0x16, 0x04, 0x0a, 0x09, 0x07, 0x15, 0x1b, 
	0x0a, 0x04, 0x16, 0x18, 0x1b, 0x15, 0x07, 0x09,
	0x01, 0x0f, 0x1d, 0x13, 0x10, 0x1e, 0x0c, 0x02, 
	0x1c, 0x12, 0x00, 0x0e, 0x0d, 0x03, 0x11, 0x1f, 
	0x17, 0x19, 0x0b, 0x05, 0x06, 0x08, 0x1a, 0x14, 
	0x0f, 0x01, 0x13, 0x1d, 0x1e, 0x10, 0x02, 0x0c, 
	0x04, 0x0a, 0x18, 0x16, 0x15, 0x1b, 0x09, 0x07, 
	0x19, 0x17, 0x05, 0x0b, 0x08, 0x06, 0x14, 0x1a, 
	0x12, 0x1c, 0x0e, 0x00, 0x03, 0x0d, 0x1f, 0x11, 
	0x14, 0x1a, 0x08, 0x06, 0x05, 0x0b, 0x19, 0x17, 
	0x1f, 0x11, 0x03, 0x0d, 0x0e, 0x00, 0x12, 0x1c, 
	0x02, 0x0c, 0x1e, 0x10, 0x13, 0x1d, 0x0f, 0x01, 
	0x09, 0x07, 0x15, 0x1b, 0x18, 0x16, 0x04, 0x0a, 
	0x11, 0x1f, 0x0d, 0x03, 0x00, 0x0e, 0x1c, 0x12, 
	0x1a, 0x14, 0x06, 0x08, 0x0b, 0x05, 0x17, 0x19, 
	0x07, 0x09, 0x1b, 0x15, 0x16, 0x18, 0x0a, 0x04, 
	0x0c, 0x02, 0x10, 0x1e, 0x1d, 0x13, 0x01, 0x0f, 
	0x1e, 0x10, 0x02, 0x0c, 0x0f, 0x01, 0x13, 0x1d, 
	0x15, 0x1b, 0x09, 0x07, 0x04, 0x0a, 0x18, 0x16, 
	0x08, 0x06, 0x14, 0x1a, 0x19, 0x17, 0x05, 0x0b, 
	0x03, 0x0d, 0x1f, 0x11, 0x12, 0x1c, 0x0e, 0x00, 
	0x1b, 0x15, 0x07, 0x09, 0x0a, 0x04, 0x16, 0x18, 
	0x12, 0x1e, 0x0c, 0x02, 0x01, 0x0f, 0x1d, 0x13, 
	0x0d, 0x03, 0x11, 0x1f, 0x1c, 0x12, 0x00, 0x0e, 
	0x06, 0x08, 0x1a, 0x14, 0x17, 0x19, 0x0b, 0x05 
};

const uint8_t decode_lut[256] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	/* 0x10 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	/* 0x20 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 
	/* 0x30 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x05, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	/* 0x40 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0x0c, 0xff, 0x0a, 0xff, 0xff, 
	/* 0x50 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0x0d, 0x01, 0xff, 
    0xff, 0xff, 0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 
	/* 0x60 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0x0b, 0x00, 0xff, 
	0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 
	/* 0x70 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	/* 0x80 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	/* 0x90 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x04, 0xff, 
	0xff, 0xff, 0x08, 0xff, 0xff, 0xff, 0xff, 0xff, 
	/* 0xa0 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0x0e, 0x02, 0xff, 
	0xff, 0xff, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 
	/* 0xb0 */
    0xff, 0xff, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	/* 0xc0 */
	0x10, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	/* 0xd0 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	/* 0xe0 */
	0x11, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	/* 0xf0 */
	0x12, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0x13, 0xff, 0xff, 0xff, 0x14, 0xff, 0xff, 0xff 
};

static uint8_t rflink_crc5(uint8_t d[])
{
	uint8_t crc;
	uint8_t idx;

	idx = 0x1f ^ (d[0] & 0xe0);
	crc = crc5lut[idx];
	idx = crc ^ d[1];
	crc = crc5lut[idx];
	idx = crc ^ d[2];
	crc = crc5lut[idx];
	idx = crc ^ d[3];
	crc = crc5lut[idx];
	crc ^= 0x1f;
	return crc & 0x1f;
}

void usart_init(void)
{
	/* Set Baud Rate */
	UBRR0 = (((F_CPU) / ((USART_BAUDRATE) * 16ul))) - 1;

	/* Set Frame Format */
	UCSR0C = ASYNCHRONOUS | PARITY_MODE | STOP_BIT | DATA_BIT;
	
	/* Enable receiver */
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	
	/* Enable rx complete interrupt */
	UCSR0B |= (1 << RXCIE0);

	/* Set PD1 as output TXD */
	DDRD |= (1 << 1);
	/* Set PD0 as input RXD */
	DDRD &= ~(1 << 0);
}

struct pkt {
	uint8_t dat[4];
};

struct {
	volatile uint8_t head;
	volatile uint8_t tail;
	uint8_t state;
	struct pkt pkt;
} rx;

#define RF_IDLE 0
#define RF_SYNC 1
#define RF_SOF  2
#define RF_EOF 11

ISR(USART_RX_vect)
{
	uint8_t nibble;
	uint8_t state;
	uint8_t c;

	c = UDR0;

	nibble = decode_lut[c];

  	state = rx.state;

	if ((nibble >= 0x11) && (nibble <= 0x13)) {
		if (state == RF_SYNC) {
			rx.state = RF_SOF; /* SOF */
		} else if (state != RF_SOF) {
			rx.state = RF_SYNC; /* SYNC */
		}
		return;
	}

	switch (state) {
	case RF_SOF:
		rx.pkt.dat[0] = nibble;
		state = 3;
		break;
		
	case 3:
		rx.pkt.dat[0] |= nibble << 4;
		state = 4;
		break;

	case 4:
		rx.pkt.dat[1] = nibble;
		state = 5;
		break;

	case 5:
		rx.pkt.dat[1] |= nibble << 4;
		state = 6;
		break;

	case 6:
		rx.pkt.dat[2] = nibble;
		state = 7;
		break;

	case 7:
		rx.pkt.dat[2] |= nibble << 4;
		state = 8;
		break;

	case 8:
		rx.pkt.dat[3] = nibble;
		state = 9;
		break;

	case 9:
		rx.pkt.dat[3] |= nibble << 4;
		rx.head++;
		state = RF_IDLE;
		break;

	default:
		rx.state = RF_IDLE;
		return;
	}

	rx.state = state;
}

uint8_t rc433_pkt_recv(uint8_t dat[])
{
	uint8_t head = rx.head;
	uint8_t tail = rx.tail;
	uint8_t d[4];
	uint8_t fsc;
	
	if (head == tail) {
		return 0;
	}

	d[0] = rx.pkt.dat[0];
	d[1] = rx.pkt.dat[1];
	d[2] = rx.pkt.dat[2];
	d[3] = rx.pkt.dat[3];

	rx.tail++;

	fsc = d[0] & 0x1f;
	
	if (rflink_crc5(d) != fsc) { 
		return 0;
	}

	dat[0] = d[0] & 0xe0;
	dat[1] = d[1];
	dat[2] = d[2];
	dat[3] = d[3];

	return 1;
}

void rc433_init(void)
{
	usart_init();
}

