#include <avr/io.h>
#include <util/delay.h>
#include "button.h"

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define ZERO_BUTTON_PIN PD4

void button_init(void) {
    DDRD &= ~(1 << ZERO_BUTTON_PIN);
    PORTD |= (1 << ZERO_BUTTON_PIN);
}

uint8_t button_zero_was_pressed(void) {
    static uint8_t last_state = 1;

    uint8_t current_state = (PIND & (1 << ZERO_BUTTON_PIN)) ? 1 : 0;

    uint8_t pressed = (last_state == 1 && current_state == 0);

    last_state = current_state;

    return pressed;
}