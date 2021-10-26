/*
 * Copyright(C) 2021 Robinson (Bob) Mittman. All Rights Reserved.
 * Licensed under the MIT license. 
 * See LICENSE file in the project root for details.
 *
 */

#ifndef __RC433_H__
#define __RC433_H__

#include <stdint.h>

void rc433_init(void);

int8_t rc433_pkt_send(uint8_t dat[]);

int8_t rc433_pkt_recv(uint8_t dat[]);

#endif /* __RC433_H__ */

