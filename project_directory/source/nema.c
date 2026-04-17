#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

// Pins
#define STEP_PIN PD2
#define DIR_PIN  PD3
#define EN_PIN   PD5

static uint8_t motor_dir = 1;

void
motor_init(void) {
    DDRD |= (1 << STEP_PIN) | (1 << DIR_PIN) | (1 << EN_PIN);

    // Enable driver (DRV8825 EN is active LOW)
    PORTD &= ~(1 << EN_PIN);

    // Set direction (1 = forward, 0 = reverse)
    PORTD |= (1 << DIR_PIN);
}

void
motor_step(void) {
    PORTD |= (1 << STEP_PIN);
    _delay_us(5);   // STEP pulse width
    PORTD &= ~(1 << STEP_PIN);
}

void
motor_set_direction(uint8_t dir) {
    motor_dir = dir ? 1 : 0;   // normalize input

    if (motor_dir) {
        PORTD |= (1 << DIR_PIN);   // HIGH = forward
    } else {
        PORTD &= ~(1 << DIR_PIN);   // LOW = reverse
    }
}

uint8_t
motor_get_direction(void) {
    return motor_dir;
}