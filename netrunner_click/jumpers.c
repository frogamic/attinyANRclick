/**
 * @file    jumpers.c
 * @author  Dominic Shelton
 * @date    3 Dec 2014
 */

#include <avr/io.h>
#include "jumpers.h"

void jumpers_init (void)
{
    // Set the direction of just the jumper pins
    JUMPER_DDR &=  ~JUMPER_MASK;
    // Enable pull up resistors on jumper pins
    JUMPER_PORT |= JUMPER_MASK;
}

uint8_t jumper_state (jumper_t j)
{
    return ~(JUMPER_PIN >> (JUMPER_OFFSET + j)) & 0x01;
}

