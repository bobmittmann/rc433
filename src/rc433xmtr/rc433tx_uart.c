/*
 * Copyright(C) 2021 Robinson (Bob) Mittman. All Rights Reserved.
 * Licensed under the MIT license. 
 * See LICENSE file in the project root for details.
 *
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h> 
#include <avr/sleep.h> 
#include "rc433.h"

#ifndef RFLINK_JOIN_FRAMES
#define RFLINK_JOIN_FRAMES 1
#endif

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

#define USART_US2TMR(_ITV_US_) (((((F_CPU) / 1024ul) * (_ITV_US_)) + \
								 500000ul) / 1000000ul)

#define USART_IDLE_ITV USART_US2TMR((15000000ul)/(USART_BAUDRATE))

static const uint8_t crc5lut[256] = {
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
	0x10, 0x1e, 0x0c, 0x02, 0x01, 0x0f, 0x1d, 0x13, 
	0x0d, 0x03, 0x11, 0x1f, 0x1c, 0x12, 0x00, 0x0e, 
	0x06, 0x08, 0x1a, 0x14, 0x17, 0x19, 0x0b, 0x05 
};

static const uint8_t encode_lut[] = {
	0x66, 0x56, 0xa6, 0x6a, 0x96, 0x36, 0x5a, 0xaa, 0x9a, 0xb2, 0x4d, 
    0x65, 0x4b, 0x55, 0xa5, 0x2d };


#define PKT_LEN 4

struct pkt {
	uint8_t dat[4];
};

struct {
	volatile uint8_t head;
	volatile uint8_t tail;
	volatile uint16_t err;
	volatile uint8_t state;
	struct pkt pkt;
} tx;

#define RF_TX_IDLE 0
#define RF_TX_SYNC1 1
#define RF_TX_SYNC2 2
#define RF_TX_SYNC3 3
#define RF_TX_SYNC4 4
#define RF_TX_EOF 12

static inline void uart_tx_set(void) 
{
	PORTD |= _BV(1);
}

static inline void uart_tx_clr(void) 
{
	PORTD &= ~_BV(1);
}

void  usart_tx_enable(void)
{
	/* enable transmitter, enable interrupts */
	UCSR0B |= ((1 << TXEN0) | (1 << UDRIE0));
}

void  usart_tx_disable(void)
{
	/* disable transmitter, disable interrupts */
	UCSR0B |= ~((1 << TXEN0) | (1 << UDRIE0));
}

static inline void usart_tmr_set(uint8_t val)
{
	/* */
	OCR2A = val;
    /* reset timer counter */
	TCNT2 = 0; 
    /* enable timer clock with prescaler = 1024 */ 
	TCCR2B = (1 << FOC2A) | (1 << CS22) | (1 << CS21) | (1 << CS20);
}

ISR(TIMER2_COMPA_vect) 
{
    /* disable timer */
	TCCR2B = (1 << FOC2A);
	/* enable transmitter enable data register empty interrupt */
	UCSR0B |= ((1 << TXEN0) | (1 << UDRIE0));
}

ISR(USART_TX_vect)
{
	if (tx.tail == tx.head) {
		/* no packet pending */ 
		uart_tx_clr();
		/* disable transmitter and TX Complete Interrupt */
		UCSR0B &= ~((1 << TXEN0) | (1 << TXCIE0));
//		led_off();
		tx.state = RF_TX_IDLE;
	} else {
		/* disable data register empty interrupt and TX Complete Interrupt */
		UCSR0B &= ~((1 << UDRIE0) | (1 << TXCIE0));
		/* schedule to restart transmitter: 4.2 ms */
		uart_tx_set();
//		led_on();
		usart_tmr_set(USART_IDLE_ITV);
		tx.state = RF_TX_SYNC2;
	}
}

ISR(USART_UDRE_vect)
{
	uint8_t tail = tx.tail;
	uint8_t cnt = tx.state;
	uint8_t d;

	switch (cnt) {
	case RF_TX_IDLE:
		if ((UCSR0B & (1 << TXEN0)) == 0) {
			/* disable data register empty interrupt */
			UCSR0B &= ~(1 << UDRIE0);
			/* schedule to restart transmitter: 4.2 ms */
			uart_tx_set();
//			led_on();
			usart_tmr_set(USART_IDLE_ITV);
			return;
		}
		UDR0 = 0xf0; /* Sync 1 */
		tx.state = RF_TX_SYNC1;
		break;

	case RF_TX_SYNC1:
		UDR0 = 0xf0; /* Sync 2 */
		tx.state = RF_TX_SYNC2;
		break;

	case RF_TX_SYNC2:
		UDR0 = 0xf0; /* Sync 3 */
		tx.state = RF_TX_SYNC3;
		break;

	case RF_TX_SYNC3:
		UDR0 = 0xf0; /* Sync 4 */
		tx.state = RF_TX_SYNC4;
		break;

	case RF_TX_SYNC4:
		d = tx.pkt.dat[0];
		UDR0 = encode_lut[d & 0x0f];
		tx.state = 5;
		break;

	case 5:
		d = tx.pkt.dat[0];
		UDR0 = encode_lut[(d >> 4) & 0x0f];
		tx.state = 6;
		break;

	case 6:
		d = tx.pkt.dat[1];
		UDR0 = encode_lut[d & 0x0f];
		tx.state = 7;
		break;

	case 7:
		d = tx.pkt.dat[1];
		UDR0 = encode_lut[(d >> 4) & 0x0f];
		tx.state = 8;
		break;

	case 8:
		d = tx.pkt.dat[2];
		UDR0 = encode_lut[d & 0x0f];
		tx.state = 9;
		break;

	case 9:
		d = tx.pkt.dat[2];
		UDR0 = encode_lut[(d >> 4) & 0x0f];
		tx.state = 10;
		break;

	case 10:
		d = tx.pkt.dat[3];
		UDR0 = encode_lut[d & 0x0f];
		tx.state = 11;
		break;

	case 11:
		d = tx.pkt.dat[3];
		UDR0 = encode_lut[(d >> 4) & 0x0f];
		tail++;
		tx.tail = tail;
		tx.state = RF_TX_EOF;
		break;

	case RF_TX_EOF:
#if (RFLINK_JOIN_FRAMES)
		if (tx.tail != tx.head) {
			tx.state = RF_TX_SYNC2;
		} else
#endif
		{
			/* disable the Data Register Empty Interrupt */
			UCSR0B &= ~(1 << UDRIE0);
			/* enable the Tx Complete Interrupt */
			UCSR0B |= (1 << TXCIE0);
		}
		break;
	}
}

/* */
int8_t rc433_pkt_send(uint8_t dat[])
{
	uint8_t head = tx.head;
	uint8_t crc;
	uint8_t idx;
	uint8_t d[4];
	
	d[0] = dat[0];
	d[1] = dat[1];
	d[2] = dat[2];
	d[3] = dat[3];

	if (head != tx.tail) {
		return 0;
	}

	d[0] &= 0xe0;
	idx = 0x1f ^ d[0];
	crc = crc5lut[idx];
	idx = crc ^ d[1];
	crc = crc5lut[idx];
	idx = crc ^ d[2];
	crc = crc5lut[idx];
	idx = crc ^ d[3];
	crc = crc5lut[idx];
	crc ^= 0x1f;
	tx.pkt.dat[0] = d[0] | (crc & 0x1f);
	tx.pkt.dat[1] = d[1];
	tx.pkt.dat[2] = d[2];
	tx.pkt.dat[3] = d[3];

	/* signal pending */
	tx.head = head + 1;
	
	/* enable the Data Register Empty Interrupt */
	UCSR0B |= (1 << UDRIE0);

	return 1;
}

/* */
void rc433_init(void)
{
	/* Set PD1 as output TXD */
	DDRD |= (1 << 1);

	/* Timer 2 in CTC mode */
	TCCR2A = (1 << WGM21);
	/* enable Timer 2 Match A Interrupts */
	TIMSK2 = (1 << OCIE2A);
    /* reset timer counter */
	TCNT2 = 0; 
	/* */
	OCR2A = 0xff;
	/* */
	TCCR2B = (1 << FOC2A);

	/* Set Baud Rate */
	UBRR0 = (((F_CPU) / ((USART_BAUDRATE) * 16ul))) - 1;
	/* Set Frame Format */
	UCSR0C = ASYNCHRONOUS | PARITY_MODE | STOP_BIT | DATA_BIT;
	/* Enable receiver only */
	UCSR0B = (1 << RXEN0);
	/* Enable rx complete interrupt */
	UCSR0B |= (1 << RXCIE0);
}

