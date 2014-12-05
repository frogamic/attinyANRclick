/**
 * @file    buttons.c
 * @author  Dominic Shelton
 * @date    2 Dec 2014
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "buttons.h"

static volatile state_t g_buttons[BUTTON_NUM] = {0};
volatile uint8_t g_flash = 0;

void buttons_init (void)
{
    cli ();
    TCCR0B |= 1<<CS01; // Prescaler of clk(1mhz)/8
    TIMSK0 |= 1<<TOIE0; // Interrupt 
    TCNT0 = 0;
    sei (); 
    BUTTON_DDR &= ~BUTTON_MASK;
    BUTTON_PORT |= BUTTON_MASK;
}

uint8_t buttons_state (button_t b, state_t s)
{
    cli();
    if (g_buttons[b] & s)
    {
        // Clear the gonedown state
        if (s == STATE_GONEDOWN)
            g_buttons[b] &= ~STATE_GONEDOWN;
        sei();
        return 1;
    }
    sei();
    return 0;
}

ISR (TIM0_OVF_vect)
{
    // Super hacky flasher plz ignore
    g_flash++;

    static uint8_t button[BUTTON_NUM] = {0};
    uint8_t value = BUTTON_PIN;

    for (int i = 0; i < BUTTON_NUM; i++)
    {
        g_buttons[i] = 0;
        if (!(value & 0x1))
            button[i]++;
        else
            button[i] = 0;

        if (button[i] == BUTTON_DEBOUNCE)
            g_buttons[i] |= STATE_GONEDOWN;
        else if (button[i] > BUTTON_DEBOUNCE)
        {
            g_buttons[i] |= STATE_HELD;
            button[i] = BUTTON_DEBOUNCE + 1;
        }

        value = value >> 1;
    }
}

