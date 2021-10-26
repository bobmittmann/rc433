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

#define EV_TMR0  (1 << 0)
#define EV_ENC0 (1 << 1)
#define EV_ENC1 (1 << 2)
#define EV_SW1  (1 << 3)
#define EV_SW2  (1 << 4)
#define EV_TMR1  (1 << 5)

#define ENCODER0_MIN -12
#define ENCODER0_MAX 12

#define ENCODER1_MIN 0
#define ENCODER1_MAX 15

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

uint8_t io_sw1_get(void);

uint8_t io_sw2_get(void);

int8_t io_encoder0_get(void);

int8_t io_encoder1_get(void);

void io_encoder0_set(int8_t val);

void io_encoder1_set(int8_t val);

#endif /* __IO_H__ */

