/**
 * @file    buttons.c
 * @author  Dominic Shelton
 * @date    2 Dec 2014
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "buttons.h"

#define BUTTON_PORT PORTB
#define BUTTON_DDR DDRB
#define BUTTON_PIN PINB
#define BUTTON_NUM 3
#define BUTTON_MASK 0x7
#define BUTTON_DEBOUNCE 12

static volatile state_t g_buttons[BUTTON_NUM] = {0};
volatile uint8_t g_flash = 0;

void buttons_init (void)
{
    // Setup timer0 for interrupt polling of buttons.
    // Disable interrupts during timer setup.
    cli ();
    // Set timer0 prescaler to clock(1mhz)/8 (125khz).
    TCCR0B |= 1<<CS01;
    // Interrupt on timer0 overflow.
    TIMSK0 |= 1<<TOIE0;
    // Initialise the timer to 0.
    TCNT0 = 0;
    // Reenable interrupts.
    sei (); 
    // Set button pins to input.
    BUTTON_DDR &= ~BUTTON_MASK;
    // Enable pullup resistor on button pins.
    BUTTON_PORT |= BUTTON_MASK;
}

uint8_t button_state (button_t b, state_t s)
{
    // Disable interrupts while polling buttons.
    cli();
    // Create a local copy of the button state.
    state_t button = g_buttons[b];
    // Reenable interrupts.
    sei();

    if (button & s)
    {
        // The button has the requested state.
        // If the state was gonedown, clear it for next time.
        if (s == STATE_GONEDOWN)
            g_buttons[b] &= ~STATE_GONEDOWN;
        // The button had the state.
        return 1;
    }
    // The button does not have state.
    return 0;
}

ISR (TIM0_OVF_vect)
{
    // Super hacky flasher plz ignore
    g_flash++;

    // One variable per button to store the number of times the button was
    // polled in the down state.
    static uint8_t button[BUTTON_NUM] = {0};
    // Get the state from the pins.
    uint8_t value = BUTTON_PIN;

    // Iterate through the buttons
    for (int i = 0; i < BUTTON_NUM; i++)
    {
        if (!(value & 0x1))
        {
            // Increment the number of consecutive polls the button was held
            // for.
            button[i]++;
        }
        else
        {
            // Reset the number of consecutive polls the button was held for.
            button[i] = 0;
        }

        // If button was held for correct number of polls it is considered
        // down.
        if (button[i] == BUTTON_DEBOUNCE)
            g_buttons[i] |= STATE_GONEDOWN;
        else if (button[i] > BUTTON_DEBOUNCE)
        {
            // If button was held down longer it is considered held.
            g_buttons[i] |= STATE_HELD;
            button[i] = BUTTON_DEBOUNCE;
        }
        else
        {
            // Clear the held state when button released.
            g_buttons[i] &= ~STATE_HELD;
        }

        // Shift the pin values to get the next button.
        value = value >> 1;
    }
}

