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
#define LED_DIM_MASK 0x06

#define START 0x07 // Starting LED setting (7 = 00000111)
#define FLASHRATE 6 // Flashing rate during setting total clicks

#define JUMP_WRAP 0x01 // Pin0, i.e. left button
#define JUMP_MODE 0x04 // Pin2, i.e. right button
//#define JUMP_MODE 0x08 // Pin3, i.e. 3-pin jumper

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

/**
 * @brief   puts the micro to sleep for half a second
 */
void sleep (void)
{
    // Turn off all LEDs
    LED_PORT = 0;
    // Enable watchdog interrupt and set prescaler for ~~0.5~~ 0.25s.
    WDTCSR |= (1 << WDIE) | /*(1 << WDP0) |*/ (1 << WDP2);
    set_sleep_mode (SLEEP_MODE_PWR_DOWN);
    sleep_mode ();
}

ISR (WATCHDOG_vect)
{
    // Disable the watchdog interrupt again.
    WDTCSR &= !(1 << WDIE);
    // Spoof a button press to advance clicks.
    button_add_press (BUT_CLICK);
}


int main (void)
{
    // Disable Timer1, USI & ADC.
    PRR |= (1 << PRTIM1) & (1 << PRUSI) & (1 << PRADC);

    jumpers_init ();

    buttons_init ();

    // Set the LEDs to output
    LED_DDR = LED_MASK;

    // Set the waiting flag to 0 to indicate we are not waiting for the turn
    uint8_t waiting = 0;
    // Set the countup value based on the state of the mode jumper.
    uint8_t countup = jumper_state (JUMP_MODE);
    // Set the mimimum value based on the state of the mode jumper.
    uint8_t autostart = !jumper_state (JUMP_WRAP);
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

        // Set mimimum to 1 so that the permanant clicks cannot go below this
        uint8_t minimum = 0x01;
        // Choose the value to be changed.
        uint8_t* changed = &reset;
        // Choose the value to wrap to.
        uint8_t wrap = minimum;

        // Get the input
        if (mode == MODE_PERM || !countup)
        {
            // We are counting down, or changing the starting clicks per turn.
            if (mode == MODE_NORM)
            {
                // We are counting down, set the changing value and wrap.
                changed = &value;
                wrap = reset;
                minimum = 0;
            }

            // Shift the changing value depending on the button pressed.
            if (button_state (BUT_CLICK, STATE_GONEDOWN))
                shift_left (changed, minimum, wrap);
            if (button_state (BUT_ADD, STATE_GONEDOWN))
                // In these 2 modes we never wrap when shifting right.
                shift_right (changed, LED_MASK, LED_MASK);
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
                shift_right (&maxvalue, LED_MASK, LED_MASK);
            if (button_state (BUT_CLICK, STATE_GONEDOWN))
            {
                shift_right (&value, maxvalue, 0);

                // If we have wrapped then the max value for the next turn must
                // be reset for next turn.
                if (value == 0)
                    maxvalue = reset;
            }
        }

        // Set the LEDs.
        if (mode == MODE_NORM)
        {
            if (!countup && waiting)
            {
                // Opponents turn, dim LEDs
                LED_PORT = !(g_flash & LED_DIM_MASK) ? value : 0;
                if (value != reset)
                    waiting = 0;
            }
            else
            {
                // Mode is normal, display the value.
                LED_PORT = value;
            }
            if (countup)
            {
                // If counting up, display the remaining clicks dimly.
                LED_PORT |= !(g_flash & LED_DIM_MASK) ? maxvalue : 0;
            }
            else
            {
                // Counting down, display total clicks dimly
                LED_PORT |= !(g_flash & LED_DIM_MASK) ? reset : 0;
            }
        }
        else
        {
            // We are changing the click reset value for new turns, display it.
            LED_PORT = ((g_flash >> FLASHRATE) & 0x1) ? reset : 0;
            // Since you would only do this on your turn, clear the waiting.
            waiting = 0;
        }

        // If we are autostarting turns and the turn has ended power down
        // briefly before starting the next turn.
        if (autostart && !countup && value == 0)
        {
            waiting = 1;
            sleep ();
        }
    }

    return 0;

}

