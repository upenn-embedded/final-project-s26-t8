#include <avr/io.h>
#include <stdint.h>

void
adc_init(void) {
    /* AVcc reference, ADC0 selected */
    ADMUX = (1 << REFS0);

    /* Enable ADC, prescaler = 128 for 16 MHz -> 125 kHz ADC clock */
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    /* PC0 input */
    DDRC &= ~(1 << PC0);
}

uint16_t
adc_read(void) {
    ADMUX = (ADMUX & 0xF0) | 0x00; /* ADC0 */
    ADCSRA |= (1 << ADSC);         /* start conversion */
    while (ADCSRA & (1 << ADSC))
        ; /* wait */
    return ADC;
}