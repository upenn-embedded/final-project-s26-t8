#define F_CPU 16000000UL

#include "button.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

// using PB0 (PCINT0)
#define BUTTON_PIN PB1

// state must be volatile (used in ISR)
static volatile system_state_t state = STATE_ZEROING;

volatile uint8_t button_event = 0;

void
button_init(void) {

    // input + pull-up
    DDRB &= ~(1 << BUTTON_PIN);
    PORTB |= (1 << BUTTON_PIN);

    // enable pin change interrupt group 0 (PORTB)
    PCICR |= (1 << PCIE0);

    // enable interrupt for PB0 (PCINT0)
    PCMSK0 |= (1 << PCINT1);
}

system_state_t
button_get_state(void) {

    static uint32_t last_time = 0;

    if (button_event) {

        _delay_ms(10);   // now it's safe

        if (!(PINB & (1 << BUTTON_PIN))) {

            // debounce success ? update state
            if (state == STATE_ZEROING) {
                state = STATE_SETTING;
            } else if (state == STATE_SETTING) {
                state = STATE_RUNNING;
            } else {
                state = STATE_ZEROING;
            }
        }

        button_event = 0;
    }

    return state;
}

const char *
state_to_string(system_state_t s) {
    switch (s) {
    case STATE_ZEROING:
        return "ZEROING";
    case STATE_SETTING:
        return "SETTING";
    case STATE_RUNNING:
        return "RUNNING";
    default:
        return "UNKNOWN";
    }
}

// ===== INTERRUPT HANDLER =====

ISR(PCINT0_vect) {
    static uint8_t last = 1;
    uint8_t curr = (PINB & (1 << BUTTON_PIN)) ? 1 : 0;

    if (last == 1 && curr == 0) {
        button_event = 1;   // just flag it
    }

    last = curr;
}