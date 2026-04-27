#define F_CPU 16000000UL

#include "adc.h"
#include "button.h"
#include "joystick.h"
#include "nema.h"
#include "uart.h"
#include <avr/interrupt.h>
#include <avr/io.h>

int
main(void) {

    motor_init();
    uart_init();
    adc_init();
    button_init();

    sei();   // ? enable global interrupts

    system_state_t state = STATE_ZEROING;
    system_state_t prev_state = STATE_ZEROING;

    while (1) {

        state = button_get_state();

        // ===== STATE ENTRY (runs once) =====
        if (state != prev_state) {

            switch (state) {

            case STATE_ZEROING:
                // optional: do something when entering zeroing
                break;

            case STATE_SETTING:
                motor_set_position(0);   // ? reset here
                break;

            case STATE_RUNNING:
                // optional
                break;
            }
        }

        // ===== STATE BEHAVIOR (runs continuously) =====
        switch (state) {

        case STATE_ZEROING:
            motor_handle_zeroing();
            break;

        case STATE_SETTING:
            motor_handle_setting();
            break;

        case STATE_RUNNING:
            motor_handle_running();
            break;
        }

        motor_print_y();

        prev_state = state;   // update at end
    }
}