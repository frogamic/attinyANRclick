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

#define START 0x07
#define FLASHRATE 6

typedef enum {MODE_NORM, MODE_PERM} mode_t;

void increase (uint8_t *value, uint8_t max, uint8_t wrap)
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

void decrease (uint8_t *value, uint8_t wrap)
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
    // Initialise the jumpers
    jumpers_init ();

    uint8_t countup = !jumper_state (JUMP_MODE);
    uint8_t reset = START;
    uint8_t value = reset;
    uint8_t maxvalue = LED_MASK;
    uint8_t setup = 0;
    mode_t mode = MODE_NORM;

    // Setup port C for LEDs
    LED_DDR = LED_MASK;

    // Initialise the buttons
    buttons_init ();

    // Loop forever
    while (1)
    {
        if (setup == 2)
            mode = buttons_state (BUT_MODE, STATE_HELD);
        else if (setup == 0)
        {
            mode = MODE_PERM;
            if (buttons_state (BUT_MODE, STATE_HELD))
            {
                setup = 1;
            }
        }
        else if (setup == 1)
        {
            if (!buttons_state (BUT_MODE, STATE_HELD))
            {
                setup = 2;
                if (!countup)
                    value = reset;
                else
                {
                    maxvalue = reset;
                    value = 0;
                }
            }
        }
        uint8_t* changed = &reset;
        uint8_t wrap = 0;

        if (mode == MODE_PERM || !countup)
        {
            if (mode == MODE_NORM)
            {
                changed = &value;
                wrap = reset;
            }
            if (buttons_state (BUT_LESS, STATE_GONEDOWN))
                decrease (changed, wrap);
            if (buttons_state (BUT_MORE, STATE_GONEDOWN))
                increase (changed, LED_MASK, 0);
            if (value == 0 && countup)
            {
                maxvalue = reset;
            }
        }
        else
        {
            if (buttons_state (BUT_MORE, STATE_GONEDOWN))
                increase (&maxvalue, LED_MASK, 0);
            if (buttons_state (BUT_LESS, STATE_GONEDOWN))
            {
                increase (&value, maxvalue, 1);
                if (value == 0)
                    maxvalue = reset;
            }
        }

        if (mode == MODE_NORM)
        {
            LED_PORT = value;
            if (countup)
                LED_PORT |= !(g_flash & 0x7) ? maxvalue : 0;
        }
        else
            LED_PORT = ((g_flash >> FLASHRATE) & 0x1) ? reset : 0;

    }

    return 0;

}

