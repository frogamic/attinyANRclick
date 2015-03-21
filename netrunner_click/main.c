/**
 * @file    main.c
 * @author  Dominic Shelton
 * @date    29 Nov 2014
 */

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "buttons.h"
#include "jumpers.h"

#define LED_PORT PORTA // LED hardware port
#define LED_DDR DDRA // LED hardware Data Direction Register
#define LED_MASK 0xff // Total LEDs possible, (ff = 11111111)
// The flash value is anded with this, if true the LEDs light.
#define LED_DIM_MASK 0x07

#define START 0x0f // Starting LED setting (f = 00001111 i.e. 4 clicks)
#define FLASH_RATE 6 // Flashing rate during setting total clicks
#define ROLLOVER_TIME 3 // The number of 'flashes' before rolling over

#define JUMP_WRAP 0x01 // Pin0, i.e. left button
//#define JUMP_WRAP 0x08 // Pin3, i.e. 3-pin jumper

typedef enum {MODE_NORM, MODE_PERM} mode_t;

/**
 * @brief   Shifts LEDs to the right by 1, padding the left most bit, and
 *          wrapping as desired.
 * @param   value   A pointer to the 8 bit value to be shifted.
 * @param   max     A bitmask of the max value. If a shift is attempted on this
 *                  value (or greater), value will not be shifted and a wrap may
 *                  occur.
 * @param   wrap    A bitmask of the value to wrap to once value reaches max.
 */
void shift_right (uint8_t *value, uint8_t max, uint8_t wrap)
{
    if ((*value & max) != max)
    {
        *value = (0x01 | (*value << 1)) & max;
    }
    else
    {
        *value = wrap;
    }
}

/**
 * @brief   Shifts the LEDs to the left by 1, wrapping as desired.
 * @param   value   A pointer to the 8 bit value to be shifted.
 * @param   min     A bitmask of the miminimum allowable value before wrapping.
 * @param   wrap    A bitmask of the value to wrap to once value reaches min.
 */
void shift_left (uint8_t *value, uint8_t min, uint8_t wrap)
{
    if (*value != min)
    {
        *value = *value >> 1;
    }
    else
    {
        *value = wrap;
    }
}

int main (void)
{
    // Disable Timer1, USI & ADC.
    PRR |= (1 << PRTIM1) | (1 << PRUSI) | (1 << PRADC);

    jumpers_init ();

    buttons_init ();

    // Set the LEDs to output
    LED_DDR = LED_MASK;

    // Set the flag to automatically advance at the end of a turn.
    uint8_t autorollover = !jumper_state (JUMP_WRAP);
    // Set the initial number of clicks to reset to each turn.
    uint8_t reset = START;
    // Set the initial number of clicks to start with.
    uint8_t value = reset;
    // Start the setup process on power on.
    uint8_t setup = 0;
    // Set the initial value for rollover after turn finishes
    uint8_t rollover = 0;
    // Value used to prevent rollover counter from decrementing too much.
    uint8_t rollover_last = 0;
    // Set the initial system state to permanent change mode.
    mode_t mode = MODE_PERM;

    // Loop forever
    while (1)
    {
        // setup mode we change the value with input.
        uint8_t* changing = &reset;
        // Can't permanently have fewer than 1 click.
        uint8_t minimum = 1;
        // No wrap when changing permanent clicks.
        uint8_t wrap = 1;

        if (setup == 0 && button_state (BUT_MODE, STATE_HELD))
        {
            // Advance the setup process if the mode button has been held for
            // the first time since starting.
            setup = 1;
        }
        else if (setup == 1 && !button_state (BUT_MODE, STATE_HELD))
        {
            // Leave the setup process if the mode button is released.
            setup = 2;

            // Set the initial value to maxvalue.
            value = reset;
        }
        else if (setup == 2)
        {
            // Setup is complete, set mode based on mode key.
            mode = button_state (BUT_MODE, STATE_HELD);
        }

        // Select the right values for the mode.
        if (mode == MODE_NORM)
        {
            // we want to change the value.
            changing = &value;
            // Minimum value of 0.
            minimum = 0;
            // Wrap to the reset value.
            wrap = reset;
        }

        // Get input
        if (button_state (BUT_ADD, STATE_GONEDOWN))
        {
            shift_right (changing, LED_MASK, LED_MASK);
            // Cancel any automated rollover.
            rollover = 0;
        }
        if (button_state (BUT_CLICK, STATE_GONEDOWN))
        {
            shift_left (changing, minimum, wrap);
            // Cancel any automated rollover.
            rollover = 0;
        }

        if (!(g_flash >> FLASH_RATE) & 0x01)
            rollover_last = 0;
        if (rollover > 0)
        {
            // Decrement rollover counter only if we have not already this 'flash'
            if ((g_flash >> FLASH_RATE) & 0x01 && rollover != rollover_last)
            {
                rollover --;
                rollover_last = rollover;
            }
            if (rollover == 0)
                value = reset;
        }
        else if (value == 0 && autorollover)
        {
            // Start turn autorollover if we are on 0 clicks and autorollover is enabled.
            rollover = ROLLOVER_TIME;
        }

        // Set the LED state.
        if (mode == MODE_PERM)
        {
            // Display the reset value flashing
            LED_PORT = ((g_flash >> FLASH_RATE) & 0x01) ? reset : 0;
        }
        else
        {
            LED_PORT = value;
            // Display the maximum clicks dimly.
            LED_PORT |= !(g_flash & LED_DIM_MASK) ? reset : 0;
        }
    }
}
