/*
 * Copyright(C) 2021 Robinson (Bob) Mittman. All Rights Reserved.
 * Licensed under the MIT license. 
 * See LICENSE file in the project root for details.
 *
 */

#include "io.h"
#include "rc433.h"
#include <util/delay.h>
#include <avr/interrupt.h> 
#include <avr/sleep.h> 

int main(void)
{
	uint8_t busy;

	io_init();
	rc433_init();

	led_on();
	_delay_ms(50);
	led_off();
	_delay_ms(200);
	led_on();
	_delay_ms(50);
	led_off();

	/* Enable Global Interrupts */
	sei();

#if 0
	uart_puts("\r\n+++\r\n=== RF433 Sniffer ===\r\n");
	uart_drain();
#endif

	busy = 255;
	io_tmr0_set(255);

	while (1) {
		uint8_t dat[4];
		uint8_t ev;

		sleep_mode();

		if (rc433_pkt_recv(dat)) {
			uint8_t op;
			uint8_t val1;
			uint8_t val2;

			op = dat[1];
			val1 = dat[2];
			val2 = dat[3];
			(void)op;
			(void)val1;
			(void)val2;

			led_flash(50);
			io_tmr0_set(255);
			busy = 255;

			if (((op + 1) == val1) && ((val1 + 1) == val2))
				continue;

		} 

		if ((ev = io_events_get()) != 0) {
			if (ev & EV_TMR0) { 
				if (--busy == 0) {
					led_flash(250);
				} else {
					io_tmr0_set(255);
				}
			}
		}
	}
}


