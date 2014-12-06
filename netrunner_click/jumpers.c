/**
 * @file    jumpers.c
 * @author  Dominic Shelton
 * @date    3 Dec 2014
 */

#include <avr/io.h>
#include "jumpers.h"

#define JUMPER_PORT PORTB
#define JUMPER_DDR DDRB
#define JUMPER_PIN PINB
// CHANGED TO 2 FOR TESTING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define JUMPER_OFFSET 2  // TODO: CHANGE BACK     !!!!!!!
#define JUMPER_MASK 0x0f // TODO: CHANGE to 0x08  !!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void jumpers_init (void)
{
    // Set the direction of just the jumper pins
    JUMPER_DDR &=  ~JUMPER_MASK;
    // Enable pull up resistors on jumper pins
    JUMPER_PORT |= JUMPER_MASK;
}

uint8_t jumper_state (jumper_t j)
{
    return !(JUMPER_PIN >> (JUMPER_OFFSET + j)) & 0x01;
}

