/**
 * @file    main.c
 * @author  Dominic Shelton
 * @date    29 Nov 2014
 */

#include <avr/io.h>
#include "buttons.h"
#include "jumpers.h"

#define LED_PORT PORTB
#define LED_DDR DDRB
#define LEDS 0xff

#define START 0xe0
#define FLASHRATE 6

typedef enum {MODE_NORM, MODE_PERM} mode_t;

void increase (uint8_t *value, uint8_t max, uint8_t wrap, uint8_t pad_bit)
{
    if ((*value & max) != max)
    {
        *value = (pad_bit | (*value >> 1)) & max;
    }
    else if (wrap)
    {
        *value = 0;
    }
}

void decrease (uint8_t *value, uint8_t wrap, uint8_t pad_bit)
{
    if (*value != 0)
    {
        *value = pad_bit | (*value << 1);
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

    uint8_t reset = START | (jumper_state(JUMP_START) << 4);
    uint8_t value = reset;

    mode_t mode = MODE_NORM;

    // Setup port C for LEDs
    LED_DDR = LEDS;

    // Initialise the buttons
    buttons_init ();

    // Loop forever
    while (1)
    {
        mode = buttons_state (BUT_MODE, STATE_HELD);

        if (buttons_state (BUT_LESS, STATE_GONEDOWN))
        {
            if (mode == MODE_NORM)
            {
                decrease (&value, reset, 0);
            }
            else
            {
                decrease (&reset, 0, 0);
            }
        }

        if (buttons_state (BUT_MORE, STATE_GONEDOWN))
        {
            if (mode == MODE_NORM)
            {
                increase (&value, 0xff, 0, 0x80);
            }
            else
            {
                increase (&reset, 0xff, 0, 0x80);
            }
        }

        if (mode == MODE_NORM)
            LED_PORT = value;
        else
            LED_PORT = ((g_flash >> FLASHRATE) & 0x1) ? reset : 0;

    }

    return 0;

}

