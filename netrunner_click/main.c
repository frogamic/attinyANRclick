#include <avr/io.h>
#include "buttons.h"

#define LED_PORT PORTB
#define LED_DDR DDRB
#define LEDS 0xff

#define START 0xf0
#define FLASHRATE 6

typedef enum {MODE_NORM, MODE_PERM} mode_t;

int main (void)
{
    uint8_t reset = START;
    uint8_t value = START;
    mode_t mode = MODE_NORM;

    // Setup port C for LEDs
    LED_DDR = LEDS;
    LED_PORT = reset;

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
                if (!(value & LEDS))
                {
                    value = reset;
                }
                else
                {
                    value = (value << 1) & LEDS;
                }
            }
            else
            {
                if ((reset & LEDS))
                    reset = LEDS & (reset << 1);
            }
        }

        if (buttons_state (BUT_MORE, STATE_GONEDOWN))
        {
            if (mode == MODE_NORM)
            {
                if (!(value & 0x1))
                    value = 0x80 | (value >> 1);
            }
            else
            {
                if (!(reset & 0x1))
                    reset = 0x80 | (reset >> 1);
            }
        }

        if (mode == MODE_NORM)
            LED_PORT = value;
        else
            LED_PORT = ((g_flash >> FLASHRATE) & 0x1) ? reset : 0;

    }

    return 0;

}

