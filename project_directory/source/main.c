/* 
 * File:   main.c
 * Author: tsnevan
 *
 * Created on April 15, 2026, 3:28 AM
 */


#define F_CPU 16000000UL

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"
#include "nema.h"
#include "adc.h"
#include "joystick.h"
#include "button.h"

#define MOTOR_IDLE_DELAY_US 1000

uint16_t print_counter = 0;

typedef enum {
    STATE_ZEROING,    // find physical reference
    STATE_STARTUP,    // choose initial Y
    STATE_RUN         // normal operation
} system_state_t;

system_state_t state = STATE_ZEROING;

int main(void) {
    uart_init();
    adc_init();
    motor_init();
    button_init();
    
    printf("System initialized\r\n");

    while (1) {

        // BUTTON
        if (button_zero_was_pressed()) {

            if (state == STATE_ZEROING) {
                motor_set_position(0);
                printf("ZERO SET\r\n");
                state = STATE_STARTUP;

            } else if (state == STATE_STARTUP) {
                printf("START Y CONFIRMED\r\n");
                state = STATE_RUN;

            } else {
                motor_set_position(0);
                printf("ZEROED (RUN)\r\n");
            }
        }

        int16_t speed = joystick_get_speed();

        // ===== ZEROING =====
        if (state == STATE_ZEROING) {

            if (speed != 0) {
                joystick_apply_direction(&speed);
                motor_step_slow();
            }

            if (++print_counter > 200) {
                print_counter = 0;
                printf("[ZEROING] pos: %ld\r\n", motor_position);
            }

            continue;
        }

        // ===== STARTUP =====
        if (state == STATE_STARTUP) {

            if (speed != 0) {
                joystick_apply_direction(&speed);
                motor_step_precision();
            }

            if (++print_counter > 200) {
                print_counter = 0;
                printf("[STARTUP] y: %.2f\r\n", motor_get_y());
            }

            continue;
        }

        // ===== RUN =====
        if (state == STATE_RUN) {

            if (speed == 0) {
                motor_delay(MOTOR_IDLE_DELAY_US);
                continue;
            }

            uint16_t mag = joystick_apply_direction(&speed);

            if (!motor_can_step(mag)) {
                continue;
            }

            uint16_t delay = motor_compute_delay(mag);

            motor_step_responsive(delay);

            if (++print_counter > 100) {
                print_counter = 0;
                printf("[RUN] y: %.2f\r\n", motor_get_y());
            }
        }
    }
}