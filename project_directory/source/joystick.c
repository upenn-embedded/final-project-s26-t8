#include "joystick.h"
#include "adc.h"

// ===== JOYSTICK TUNING =====
#define JOYSTICK_CENTER   512
#define JOYSTICK_DEADZONE 30

int16_t
joystick_get_speed(void) {
    uint16_t val = adc_read();

    int16_t diff = val - JOYSTICK_CENTER;

    if (diff > -JOYSTICK_DEADZONE && diff < JOYSTICK_DEADZONE)
        return 0;

    return diff;
}

uint16_t
joystick_get_direction(int16_t speed) {
    if (speed > 0) {
        return (1);
    } else {
        return (0);
    }
}