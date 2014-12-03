/**
 * @file    jumpers.h
 * @author  Dominic Shelton
 * @date    3 Dec 2014
 */

#ifndef JUMPERS_H
#define JUMPERS_H

#define JUMPER_PORT PORTD
#define JUMPER_DDR DDRD
#define JUMPER_PIN PIND
#define JUMPER_OFFSET 3
#define JUMPER_MASK 0x8

typedef enum {JUMP_START} jumper_t;

void jumpers_init (void);

uint8_t jumper_state (jumper_t j);

#endif

