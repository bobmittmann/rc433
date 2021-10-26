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
#include <stdbool.h>

#include "io.h"
#include "rc433.h"

void led_flash(uint8_t itv)
{
	io_tmr0_set(itv);
	led_on();
}

#define FORWARD 1
#define REVERSE 2

int main(void)
{
	uint8_t dat[4];
	uint8_t mode = 0;
	uint8_t xmt = 0;
	uint8_t sw1 = 0;
	uint8_t sw2 = 0;
	uint8_t idle = 0;
	uint8_t seq = 0;

	io_init();
	rc433_init();

	dat[0] = 0;
	dat[1] = 1;
	dat[2] = 0;
	dat[3] = 0;

	/* Enable Global Interrupts */
	sei();

	mode = 3;
	xmt = 1;

	while(1) {
		uint8_t ev;

		while ((ev = io_events_get()) == 0) {
			if (xmt) {
				if (rc433_pkt_send(dat)) {
					led_flash(100);
					xmt--;
				}
			}
			sleep_mode();
		}	

		if (ev & EV_TMR0) {
			led_off();
		}

		if (ev & EV_SW2) {
			sw2 = io_sw2_get();
			if (mode != sw2) {
				mode = sw2;
				if ((sw1 = io_sw1_get())) {
					mode += 4;
				}
			}
			io_encoder0_set(0);
			io_encoder1_set(0);
			xmt = 1;
		}

		switch (mode) {
		case FORWARD:
			if ((ev & EV_SW2) || (ev & EV_ENC0) || (ev & EV_ENC1)) {
				int8_t steer = io_encoder0_get();
				int8_t speed = io_encoder1_get();
				uint8_t m_left;
				uint8_t m_right;

				if (speed > 0) {
					if (steer < 0) { 
						uint8_t rate = 16 - ((uint8_t)-steer);

						m_left = (uint8_t)(speed * (rate)) / 4;
						m_right = (uint8_t)(speed * 16) / 4;
					} else {
						uint8_t rate = 16 - ((uint8_t)steer);

						m_left = (uint8_t)(speed * 16) / 4;
						m_right = (uint8_t)(speed * (rate)) / 4;
					}

					m_left += 63;
					m_right += 63;

					if (m_left > 124)
						m_left = 124;

					if (m_right > 124)
						m_right = 124;
				} else {
					m_left = 0;
					m_right = 0;
				}

				dat[1] = 1;
				dat[2] = (int8_t)(m_left);
				dat[3] = (int8_t)(m_right);

				io_tmr1_set(255);
				xmt = 10;
				idle = 80;
			} else if (ev & EV_SW1) {
				if ((sw1 = io_sw1_get())) {

					dat[1] = 1;
					dat[2] = 0;
					dat[3] = 0;

					idle = 0;
					io_tmr1_set(0);
					io_encoder0_set(0);
					io_encoder1_set(0);
					xmt = 10;
				};
			} 

			if (ev & EV_TMR1) {
				io_tmr1_set(255);
				if (idle) {
					if (--idle == 0) {
						xmt = 10;
						idle = 80;
					}
				}
			}

			break;

		case REVERSE:
			if ((ev & EV_SW2) || (ev & EV_ENC0) || (ev & EV_ENC1)) {
				int8_t steer = io_encoder0_get();
				int8_t speed = io_encoder1_get();
				uint8_t m_left;
				uint8_t m_right;

				if (speed > 0) {
					if (steer < 0) { 
						uint8_t rate = 16 - ((uint8_t)-steer);

						m_left = (uint8_t)(speed * rate) / 4;
						m_right = (uint8_t)(speed * 16) / 4;

					} else {
						uint8_t rate = 16 - ((uint8_t)steer);

						m_left = (uint8_t)(speed * 16) / 4;
						m_right = (uint8_t)(speed * rate) / 4;
					}

					m_left += 63;
					m_right += 63;

					if (m_left > 100)
						m_left = 100;

					if (m_right > 100)
						m_right = 100;

				} else {
					m_left = 0;
					m_right = 0;
				}

				dat[1] = 1;
				dat[2] = (int8_t)(-1 * (int8_t)(m_left));
				dat[3] = (int8_t)(-1 * (int8_t)(m_right));


				io_tmr1_set(255);
				xmt = 10;
				idle = 80;
			} else if (ev & EV_SW1) {
				if ((sw1 = io_sw1_get())) {

					dat[1] = 1;
					dat[2] = 0;
					dat[3] = 0;

					idle = 0;
					io_tmr1_set(0);
					io_encoder0_set(0);
					io_encoder1_set(0);
					xmt = 10;
				};
			} 

			if (ev & EV_TMR1) {
				io_tmr1_set(255);
				if (idle) {
					if (--idle == 0) {
						xmt = 10;
						idle = 80;
					}
				}
			}
			break;

		case 3:
			if ((ev & EV_SW2) || (ev & EV_ENC0) || (ev & EV_ENC1)) {

				dat[1] = 1;
				dat[2] = 0;
				dat[3] = 0;
				idle = 0;

				io_encoder0_set(0);
				io_encoder1_set(0);

				xmt = 10;
			}

		case 0:
			if ((ev & EV_SW2) || (ev & EV_ENC0) || (ev & EV_ENC1)) {
				int8_t val;
				
				val  = io_encoder0_get();

				val *= 15;
				if (val < -127)
					val = -127;
				dat[2] = (uint8_t)val;

				val  = io_encoder1_get();

				val *= 15;
				if (val < -127)
					val = -127;
				dat[3] = (uint8_t)val;
				xmt = 1;
				dat[1] = 1;
			}
			break;

		case 5:
			if ((ev & EV_SW2) || (ev & EV_TMR1)) {
				dat[0] = 0;
				dat[1] = seq++;
				dat[2] = seq++;
				dat[3] = seq++;

				io_tmr1_set(255);
				xmt = 1;
			}
			break;
		}
	}
}

