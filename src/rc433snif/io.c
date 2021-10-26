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
	struct {
		volatile uint8_t set;
	} ev;

	volatile uint8_t tmr[2];
	volatile uint8_t led_tmr;
} io;

/* compare interrupt service routine */
ISR(TIMER2_COMPA_vect) 
{
	uint8_t ev;
	uint8_t tmr;

	dbg0_toggle();

	ev = 0;

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

	if ((tmr = io.led_tmr) != 0) {
		if (--tmr == 0)
			led_off();
		io.led_tmr = tmr;
	}

	/* set event bits */
	io.ev.set |= ev;
}

void io_init(void) 
{
	/* Set PB5 as output LED */
	/* Set PB0 as output debug */
	DDRB = (1 << 5) | (1 << 0);

	io.ev.set = 0;
	io.tmr[0] = 0;
	io.tmr[1] = 0;
	io.led_tmr = 0;

	/* Set timer CTC mode */
	TCCR2A = (1 << WGM21);
	/* enable Timer Match A Interrupts */
	TIMSK2 = (1 << OCIE2A);
    /* reset timer counter */
	TCNT2 = 0; 
	/* */
	OCR2A = IO_TMR_TOP; 
    /* enable timer clock with prescaler = 256 */ 
	TCCR2B = (1 << CS22) | (1 << CS21);
}

void io_tmr0_set(uint8_t itv)
{
	io.tmr[0] = itv;
}

void io_tmr1_set(uint8_t itv)
{
	io.tmr[1] = itv;
}

void led_flash(uint8_t itv)
{
	io.led_tmr = itv;
	led_on();
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

