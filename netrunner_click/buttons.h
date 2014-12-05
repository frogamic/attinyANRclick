/**
 * @file    buttons.h
 * @author  Dominic Shelton
 * @date    2 Dec 2014
 */

#ifndef BUTTONS_H
#define BUTTONS_H

#define BUTTON_PORT PORTB
#define BUTTON_DDR DDRB
#define BUTTON_PIN PINB
#define BUTTON_NUM 3
#define BUTTON_MASK 0x7
#define BUTTON_DEBOUNCE 12

typedef enum {BUT_LESS, BUT_MODE, BUT_MORE} button_t;
typedef enum {STATE_GONEDOWN = 0x01, STATE_HELD = 0x02} state_t;

// Sup3r h4cky flasher, plz ignore.
volatile uint8_t g_flash;

void buttons_init (void);

uint8_t buttons_state (button_t b, state_t s);

#endif

