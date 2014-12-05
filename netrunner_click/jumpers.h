/**
 * @file    jumpers.h
 * @author  Dominic Shelton
 * @date    3 Dec 2014
 */

#ifndef JUMPERS_H
#define JUMPERS_H

#define JUMPER_PORT PORTB
#define JUMPER_DDR DDRB
#define JUMPER_PIN PINB
// CHANGED TO 2 FOR TESTING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define JUMPER_OFFSET 2  // TODO: CHANGE BACK     !!!!!!!
#define JUMPER_MASK 0x0f // TODO: CHANGE to 0x08  !!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

typedef enum {JUMP_MODE} jumper_t;

void jumpers_init (void);

uint8_t jumper_state (jumper_t j);

#endif

