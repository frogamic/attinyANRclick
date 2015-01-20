/**
 * @file    jumpers.h
 * @author  Dominic Shelton
 * @date    3 Dec 2014
 */

#ifndef JUMPERS_H
#define JUMPERS_H

/**
 * @brief   Sets the jumper pins to input with pullup resistor.
 */
void jumpers_init (void);

/**
 * @brief   Returns the state of the jumper pin specified, low is 1.
 * @param   j   The jumper to check the state of.
 * @return  1 if the jumper is low, otherwise 0.
 */
uint8_t jumper_state (uint8_t j);

#endif

