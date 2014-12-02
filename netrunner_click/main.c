#include <avr/io.h>
#include <avr/interrupt.h>

#define BUTTON_PORT PORTD
#define BUTTON_DDR DDRD
#define BUTTON_PIN PIND
#define BUTTONS 3
#define BUTTON_MASK 0x7
#define DEBOUNCE 12

#define LED_PORT PORTB
#define LED_DDR DDRB
#define LEDS 0xff

#define START 0xf0
#define FLASHRATE 6

typedef enum {BUT_LESS, BUT_MODE, BUT_MORE} button_t;
typedef enum {MODE_NORM, MODE_PERM} mode_t;
typedef enum {STATE_GONEDOWN = 0x01, STATE_HELD = 0x02} state_t;

volatile state_t g_buttons[BUTTONS] = {0};
volatile uint8_t g_flash = 0;

uint8_t button_state (button_t b, state_t s)
{
    cli();
    if (g_buttons[b] & s)
    {
        if (s == STATE_GONEDOWN)
            g_buttons[b] &= STATE_HELD;
        sei();
        return 1;
    }
    sei();
    return 0;
}

int main (void)
{
    uint8_t reset = START;
    uint8_t value = START;
    mode_t mode = MODE_NORM;

    cli ();
    TCCR0 |= 1<<CS01;
    TIMSK |= 1<<TOIE0;
    TCNT0 = 0;
    sei (); 

    // Setup port C for LEDs
    LED_DDR = LEDS;
    LED_PORT = reset;

    // Setup port D for button input
    BUTTON_DDR &= 0xffff - BUTTON_MASK;
    BUTTON_PORT = BUTTON_MASK;

    // Loop forever
    while (1)
    {
        mode = button_state (BUT_MODE, STATE_HELD);

        if (button_state (BUT_LESS, STATE_GONEDOWN))
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

        if (button_state (BUT_MORE, STATE_GONEDOWN))
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

ISR (TIMER0_OVF_vect)
{
    g_flash++;
    static uint8_t button[BUTTONS] = {0};
    uint8_t value = BUTTON_PIN;

    for (int i = 0; i < BUTTONS; i++)
    {
        g_buttons[i] = 0;
        if (!(value & 0x1))
            button[i]++;
        else
            button[i] = 0;

        if (button[i] == DEBOUNCE)
            g_buttons[i] |= STATE_GONEDOWN;
        else if (button[i] > DEBOUNCE)
        {
            g_buttons[i] |= STATE_HELD;
            button[i] = DEBOUNCE + 1;
        }

        value = value >> 1;
    }
}
