/**
 * @file    main.c
 * @author  Dominic Shelton
 * @date    29 Nov 2014
 */

#include <avr/io.h>
#include "buttons.h"
#include "jumpers.h"

#define LED_PORT PORTA
#define LED_DDR DDRA
#define LED_MASK 0xff
#define LED_DIM_MASK 0x07

#define START 0x07
#define FLASHRATE 6

typedef enum {MODE_NORM, MODE_PERM} mode_t;

/**
 * @brief   Shifts a value to the right by 1 bit, padding the left most bit, and
 *          wrapping as desired.
 * @param   value   A pointer to the 8 bit value to be shifted.
 * @param   max     A bitmask of the max value. If a shift is attempted on this
 *                  value (or greater), value will not be shifted and a wrap may
 *                  occur.
 * @param   wrap    A boolean value indicating whether to wrap value to 0 when
 *                  max is reached.
 */
void shift_right (uint8_t *value, uint8_t max, uint8_t wrap)
{
    if ((*value & max) != max)
    {
        *value = (0x01 | (*value << 1)) & max;
    }
    else if (wrap)
    {
        *value = 0;
    }
}

/**
 * @brief   Shifts a value to the left by 1 bit, wrapping as desired.
 * @param   value   A pointer to the 8 bit value to be shifted.
 * @param   wrap    A bitmask of the value to wrap to once value reaches 0. A
 *                  bitmask of all 0s indicates that wrapping should not occur.
 */
void shift_left (uint8_t *value, uint8_t wrap)
{
    if (*value != 0)
    {
        *value = *value >> 1;
    }
    else if (wrap)
    {
        *value = wrap;
    }
}

int main (void)
{
    jumpers_init ();

    buttons_init ();

    LED_DDR = LED_MASK;

    // Set the countup value based on the state of the mode jumper.
    uint8_t countup = !jumper_state (JUMP_MODE);
    // Set the initial number of clicks to reset to each turn.
    uint8_t reset = START;
    // Set the initial number of clicks to start with.
    uint8_t value = reset;
    // Set the maximum value to all the LEDs.
    uint8_t maxvalue = LED_MASK;
    // Start the setup process on power on.
    uint8_t setup = 0;
    // Set the initial system state to permanant change mode.
    mode_t mode = MODE_PERM;

    // Loop forever
    while (1)
    {
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

            // Set the initial value and maxvalue based on the counting
            // direction.
            if (!countup)
                value = reset;
            else
            {
                maxvalue = reset;
                value = 0;
            }
        }
        else if (setup == 2)
        {
            // Setup is complete, set mode based on mode key.
            mode = button_state (BUT_MODE, STATE_HELD);
        }

        // Choose the value to be changed.
        uint8_t* changed = &reset;
        // Choose the value to wrap to.
        uint8_t wrap = 0;

        // Get the input
        if (mode == MODE_PERM || !countup)
        {
            // We are counting down, or changing the starting clicks per turn.
            if (mode == MODE_NORM)
            {
                // We are counting down, set the changing value and wrap.
                changed = &value;
                wrap = reset;
            }

            // Shift the changing value depending on the button pressed.
            if (button_state (BUT_CLICK, STATE_GONEDOWN))
                shift_left (changed, wrap);
            if (button_state (BUT_ADD, STATE_GONEDOWN))
                // In these 2 modes we never wrap when shifting right.
                shift_right (changed, LED_MASK, 0);
            if (value == 0 && countup)
            {
                // If we are not in turn and are counting up the max value for the
                // next turn needs to change to reflect the new reset value.
                maxvalue = reset;
            }
        }
        else
        {
            // We are are counting up and not changing the starting clicks.

            // Shift the values according to the buttons pressed.
            if (button_state (BUT_ADD, STATE_GONEDOWN))
                shift_right (&maxvalue, LED_MASK, 0);
            if (button_state (BUT_CLICK, STATE_GONEDOWN))
            {
                shift_right (&value, maxvalue, 1);

                // If we have wrapped then the max value for the next turn must
                // be reset for next turn.
                if (value == 0)
                    maxvalue = reset;
            }
        }

        // Set the LEDs.
        if (mode == MODE_NORM)
        {
            // Mode is normal, display the value.
            LED_PORT = value;
            // If counting up, display the remaining clicks dimly.
            if (countup)
                LED_PORT |= !(g_flash & LED_DIM_MASK) ? maxvalue : 0;
        }
        else
        {
            // We are changing the click reset value for new turns, display it.
            LED_PORT = ((g_flash >> FLASHRATE) & 0x1) ? reset : 0;
        }

    }

    return 0;

}

