/*
 * Copyright(C) 2021 Robinson (Bob) Mittman. All Rights Reserved.
 * Licensed under the MIT license. 
 * See LICENSE file in the project root for details.
 *
 */

#ifndef __IO_H__
#define __IO_H__

#include <avr/io.h>

#define IO_TMR_TOP 249
#define IO_TICKS_MS ((((IO_TMR_TOP) + 1l) * 256 * 1000l) / (F_CPU))

#define IO_MS2TICKS(__MS__) ((__MS__) / IO_TICKS_MS)

#define EV_TMR0 (1 << 0)
#define EV_TMR1 (1 << 0)

static inline void led_off(void) {
	PORTB &= ~(1 << 5);
}

static inline void led_on(void) {
	PORTB |= (1 << 5); 
}

static inline void led_toggle(void) {
	PORTB ^= (1 << 5); 
}

static inline void dbg0_off(void) {
	PORTB &= ~(1 << 0);
}

static inline void dbg0_on(void) {
	PORTB |= (1 << 0); 
}

static inline void dbg0_toggle(void) {
	PORTB ^= (1 << 0); 
}

static inline void dbg1_off(void) {
	PORTB &= ~(1 << 1);
}

static inline void dbg1_on(void) {
	PORTB |= (1 << 1); 
}

static inline void dbg1_toggle(void) {
	PORTB ^= (1 << 1); 
}

void io_init(void);

void  io_tmr0_set(uint8_t itv);

void  io_tmr1_set(uint8_t itv);

uint8_t io_events_get(void);

void led_flash(uint8_t itv);

#endif /* __IO_H__ */

