/**
 * @file    buttons.h
 * @author  Dominic Shelton
 * @date    2 Dec 2014
 */

#ifndef BUTTONS_H
#define BUTTONS_H

typedef enum {BUT_CLICK, BUT_MODE, BUT_ADD} button_t;
typedef enum {STATE_GONEDOWN = 0x01, STATE_HELD = 0x02} state_t;

// Sup3r h4cky flasher, plz ignore.
volatile uint8_t g_flash;

/**
 * @brief   Configure the timer and input pins for the buttons.
 */
void buttons_init (void);

/**
 * @brief   Check if the specified button has the specified state.
 * @param   b   The button to set.
 * @param   s   The desired state.
 * @return  1 if the button has the state, otherwise 0.
 */
uint8_t button_state (button_t b, state_t s);

#endif

