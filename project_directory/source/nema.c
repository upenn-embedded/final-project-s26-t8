#define F_CPU 16000000UL

#include "nema.h"
#include "joystick.h"
#include "uart.h"
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>

// Pins
#define STEP_PIN PD2
#define DIR_PIN  PD3
#define EN_PIN   PD4

#define MOTOR_MIN_DELAY 500
#define MOTOR_MAX_DELAY 3000
#define MIN_STEPS       -16000
#define MAX_STEPS       16000

static uint8_t motor_dir = 1;
static int32_t motor_position = 0;

volatile uint16_t step_period = 5000;   // timer ticks
volatile uint8_t step_enabled = 0;

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

    if (motor_dir) {
        motor_position++;
    } else {
        motor_position--;
    }
}

int32_t
motor_get_position(void) {
    return motor_position;
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

void
motor_set_position(int32_t pos) {
    motor_position = pos;
}

void
motor_delay(uint16_t us) {
    while (us--) {
        _delay_us(1);
    }
}

void
motor_idle() {
    motor_delay(1000);
}

void
motor_update() {

    //  printf("HELLO");
    // magnitude
    int16_t speed = joystick_get_speed();

    if (speed == 0) {
        motor_idle();
        PORTD |= (1 << EN_PIN);

        return;
    }

    else {
        // delay
        PORTD &= (1 << EN_PIN);
        float norm = (float) abs(speed) / 512.0f;

        // cubic curve for precision near center
        float shaped = sqrt(norm);

        uint16_t delay = MOTOR_MAX_DELAY - (uint16_t) (shaped * (MOTOR_MAX_DELAY - MOTOR_MIN_DELAY));

        if (delay < MOTOR_MIN_DELAY)
            delay = MOTOR_MIN_DELAY;

        uint16_t dir = joystick_get_direction(speed);

        motor_set_direction(dir);

        motor_step();
        motor_delay(delay);
    }
}

float
motor_get_y(void) {

    int32_t steps = motor_position;

    float range = (float) (MAX_STEPS - MIN_STEPS);

    if (range == 0)
        return 0;

    return ((steps - MIN_STEPS) / range) * 20.0f - 10.0f;
}

void
motor_handle_zeroing(void) {
    int16_t speed = joystick_get_speed();

    if (speed != 0) {
        uint16_t dir = joystick_get_direction(speed);

        motor_set_direction(dir);
        motor_step();
        motor_delay(1500);   // slow + precise
    } else {
        motor_idle();
    }
}

void
motor_handle_setting(void) {
    int16_t speed = joystick_get_speed();

    if (speed != 0) {
        uint16_t dir = joystick_get_direction(speed);

        motor_set_direction(dir);

        motor_step();
        motor_delay(1200);   // slightly faster
    } else {
        motor_idle();
    }
}

void
motor_handle_running(void) {
    motor_update();   // your existing logic
}

void
motor_print_y(void) {
    static uint16_t counter = 0;

    counter++;

    if (counter >= 200) {   // adjust for print rate
        counter = 0;

        float y = motor_get_y();
        printf("Y: %.2f\r\n", y);
        printf("motor position: %d\n", motor_position);
    }
}