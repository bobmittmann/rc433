/*
 * Copyright(C) 2021 Robinson (Bob) Mittman. All Rights Reserved.
 * Licensed under the MIT license. 
 * See LICENSE file in the project root for details.
 *
 */

#include "io.h"
#include <avr/interrupt.h> 
#include <util/atomic.h>

struct {
	volatile uint8_t din[2];

	struct {
		volatile uint8_t set;
		volatile uint8_t msk;
	} ev;

	struct {
		uint8_t code;
		int8_t val;
	} enc[2];

	volatile uint8_t tmr[2];
	volatile uint8_t sw2;
	volatile uint8_t sw1;
} io;


/* TIMER0 compare interrupt service routine */
ISR(TIMER0_COMPA_vect) 
{
	uint8_t dc;
	uint8_t dd;
	uint8_t d2;
	uint8_t d1;
	uint8_t d0;
	uint8_t t;
	uint8_t ev;
	uint8_t tmr;

	/* read PORTC pins */
	dc = PINC;
	dd = PIND;
	/* combine all inputs */
	d0 = (dc & 0xf) | ((dd << 2) & 0xf0);        
	/* filter */
	d1 = io.din[0];
	d2 = io.din[1];
	io.din[0] = d0;
	io.din[1] = d1;
	/* Compute transitions */
	t = ~(d0 ^ d1) & (d1 ^ d2);

	ev = 0;

	if (d0 & (1 << 4)) 
		dbg0_on();
	else
		dbg0_off();

	if (d0 & (1 << 5)) 
		dbg1_on();
	else
		dbg1_off();

	if (t) {
		/* process transition */ 
		uint8_t code;
		int8_t val;

		val = io.enc[0].val;
		code = io.enc[0].code;

		code = ((code << 2) & 0x0c) | ((d0 >> 4) & 0x03);
		if ((code == 0x03) && (val > ENCODER0_MIN)) {
			val--;
			ev |= EV_ENC0; 
		} else if ((code == 0x0c) && (val < ENCODER0_MAX)) {
			val++;
			ev |= EV_ENC0; 
		}

		io.enc[0].code = code;
		io.enc[0].val = val;

		val = io.enc[1].val;
		code = io.enc[1].code;

		code = ((code << 2) & 0x0c) | ((d0 >> 6) & 0x03);
		if ((code == 0x03) && (val > ENCODER1_MIN)) {
			val--;
			ev |= EV_ENC1; 
		} else if ((code == 0x0c) && (val < ENCODER1_MAX)) {
			val++;
			ev |= EV_ENC1; 
		}

		io.enc[1].code = code;
		io.enc[1].val = val;



		if (t & (1 << 2)) {
			if (d0 & (1 << 2)) {
				io.sw1 = 0;
			} else {
				io.sw1 = 1;
			}
			ev |= EV_SW1; 
		}

		if (t & 0x03) {
			switch (d0 & 0x03) {
			case 1:
				io.sw2 = 1;
				break;
			case 2:
				io.sw2 = 2;
				break;
			case 3:
				io.sw2 = 3;
				break;
			}
			ev |= EV_SW2; 
		}

	}

	if ((tmr = io.tmr[0]) != 0) {
		if (--tmr == 0)
			ev |= EV_TMR0; 
		io.tmr[0] = tmr;
	}

	if ((tmr = io.tmr[1]) != 0) {
		if (--tmr == 0)
			ev |= EV_TMR1; 
		io.tmr[1] = tmr;
	}

	/* set event bits */
	io.ev.set |= ev;
}

uint8_t io_sw1_get(void)
{
	return io.sw1;
}

uint8_t io_sw2_get(void)
{
	return io.sw2;
}

int8_t io_encoder0_get(void)
{
	return io.enc[0].val;
}

int8_t io_encoder1_get(void)
{
	return io.enc[1].val;
}

void io_encoder0_set(int8_t val)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		io.enc[0].val = val;
	}
}

void io_encoder1_set(int8_t val)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		io.enc[1].val = val;
	}
}

void io_init(void) 
{
	uint8_t c;
	uint8_t d;

	/* Set PB5 as output LED */
	/* Set PB0 and PB1 as output debug */
	DDRB = (1 << 5) | (1 << 0) | (1 << 1);
	/* configure PORTC inputs */
	DDRC = ~0x0f;      
	/* enable PORTC pull-ups */
	PORTC = 0x0f;    
	/* configure PORTD inputs */
	DDRD = ~0x3c;      
	/* enable PORTD pull-ups */
	PORTD = 0x3c;    
	/* read PORTC pins  state*/
	c = PINC;   
	/* read PORTD pins state */
	d = PIND;   
	/* combine all inputs */
	d = (c & 0xf) | ((d << 2) & 0xf0);        
	/* initializer debouncing filter state */
	io.din[0] = d;
	io.din[1] = d;

	io.ev.set = 0;
	io.ev.msk = 0;
	io.tmr[0] = 0;
	io.tmr[1] = 0;

	io.enc[0].val = (ENCODER0_MAX - ENCODER0_MIN) / 2;
	io.enc[0].code = 0;

	io.enc[1].val = (ENCODER1_MAX - ENCODER1_MIN) / 2;
	io.enc[1].code = 0;

	/* Set timer CTC mode */
	TCCR0A = (1 << WGM01);// | (1 << WGM00);    
	/* enable Timer Match A Interrupts */
	TIMSK0 = (1 << OCIE0A);
    /* reset timer counter */
	TCNT0 = 0; 
	/* */
	OCR0A = IO_TMR_TOP;
    /* enable timer clock with prescaler = 1024 */ 
	//TCCR0B = (1 << FOC0A) | (1 << CS02) | (1 << CS00);
    /* enable timer clock with prescaler = 256 */ 
	TCCR0B = (1 << FOC0A) | (1 << CS01);
}

void io_tmr0_set(uint8_t itv)
{
	io.tmr[0] = itv;
}

void io_tmr1_set(uint8_t itv)
{
	io.tmr[1] = itv;
}

uint8_t io_events_get(void)
{
	uint8_t set;

	/* acknowledge events */
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
   //cli();
		set = io.ev.set;
		io.ev.set = 0;
   //sei();
	}

	/* return events bitmap */
	return set;
}

