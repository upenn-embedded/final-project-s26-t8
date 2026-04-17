#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "nema.h"

// ===== STEP MODE TUNING =====
#define MOTOR_DELAY_SLOW_US       2000
#define MOTOR_DELAY_PRECISION_US  1500

// ===== RESPONSIVE STEPPING =====
#define MOTOR_DELAY_CHUNK_US      200

// ===== POSITION SCALING PARAMETERS =====
static int32_t min_steps = -10000;
static int32_t max_steps = 10000;
static uint8_t motor_dir = 1;  // 1 = forward, 0 = reverse

// ===== MOTOR TUNING PARAMETERS =====
#define MOTOR_MIN_DELAY_US 200
#define MOTOR_MAX_DELAY_US 3000
#define MOTOR_STEP_PULSE_US 5
#define MOTOR_SPEED_SCALE 3

#define STEP_PIN PD2
#define DIR_PIN  PD3

volatile int32_t motor_position = 0;

void motor_init(void) {
    DDRD |= (1 << STEP_PIN) | (1 << DIR_PIN);
}

void motor_step(void) {
    PORTD |= (1 << STEP_PIN);
    _delay_us(MOTOR_STEP_PULSE_US);
    PORTD &= ~(1 << STEP_PIN);

    if (motor_dir)
        motor_position++;
    else
        motor_position--;
}

uint16_t motor_compute_delay(int16_t speed) {
    // speed should already be positive
    uint16_t delay = MOTOR_MAX_DELAY_US - (MOTOR_SPEED_SCALE * speed);

    if (delay < MOTOR_MIN_DELAY_US)
        delay = MOTOR_MIN_DELAY_US;

    return delay;
}

uint8_t motor_get_direction(void) {
    return motor_dir;
}

uint8_t motor_can_step(int16_t speed) {
    if (motor_position >= max_steps && speed > 0)
        return 0;

    if (motor_position <= min_steps && speed < 0)
        return 0;

    return 1;
}

int32_t motor_get_min_steps(void) {
    return min_steps;
}

int32_t motor_get_max_steps(void) {
    return max_steps;
}

void motor_set_position(int32_t pos) {
    motor_position = pos;
}

void motor_delay(uint16_t us) {
    while (us--) _delay_us(1);
}

void motor_set_direction(uint8_t dir) {
    motor_dir = dir;

    if (dir)
        PORTD |= (1 << DIR_PIN);
    else
        PORTD &= ~(1 << DIR_PIN);
}

float motor_get_y(void) {
    int32_t steps = motor_position;

    // clamp
    if (steps < min_steps) steps = min_steps;
    if (steps > max_steps) steps = max_steps;

    float range = (float)(max_steps - min_steps);

    if (range == 0)
        return 0;

    return ((steps - min_steps) / range) * 20.0f - 10.0f;
}

void motor_step_slow(void) {
    motor_step();
    motor_delay(MOTOR_DELAY_SLOW_US);
}

void motor_step_precision(void) {
    motor_step();
    motor_delay(MOTOR_DELAY_PRECISION_US);
}

void motor_step_responsive(uint16_t delay) {
    motor_step();

    for (uint16_t i = 0; i < delay; i += MOTOR_DELAY_CHUNK_US) {
        _delay_us(MOTOR_DELAY_CHUNK_US);
    }
}
