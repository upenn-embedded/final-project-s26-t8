#include "joystick.h"
#include "adc.h"
#include "nema.h"

// ===== JOYSTICK TUNING =====
#define JOYSTICK_CENTER   512
#define JOYSTICK_DEADZONE 50

int16_t
joystick_get_speed(void) {
    uint16_t val = adc_read(1);

    int16_t diff = val - JOYSTICK_CENTER;

    if (diff > -JOYSTICK_DEADZONE && diff < JOYSTICK_DEADZONE)
        return 0;

    return diff;
}

uint16_t
joystick_apply_direction(int16_t *speed) {
    if (*speed > 0) {
        motor_set_direction(1);
        return *speed;
    } else {
        motor_set_direction(0);
        return -(*speed);
    }
}