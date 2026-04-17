/* 
 * File:   main.c
 * Author: tsnevan
 *
 * Created on April 15, 2026, 3:28 AM
 */

#define F_CPU 16000000UL

#include "nema.h"
#include <avr/io.h>
#include <util/delay.h>

int
main(void) {
    motor_init();

    // Optional: set direction once
    motor_set_direction(1);   // 1 = forward, 0 = reverse

    while (1) {
        motor_step();
        _delay_us(500);   // adjust for speed
    }
}