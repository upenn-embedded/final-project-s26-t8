/* 
 * File:   encoder.c
 * Author: sebastian
 *
 * Created on April 10, 2026, 2:55 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include "uart.h"

#define F_CPU 16000000UL


#define DDR_SPI DDRB

#define CS DDB2

#define DD_MOSI DDB3

#define DD_MISO DDB4

#define DD_SCK DDB5

#define CS DDB2

// SPI_ functions yoinked from reference manual example

void SPI_MasterInit(void)
{
/* Set MOSI and SCK output, all others input */
DDR_SPI = (1<<DD_MOSI)|(1<<DD_SCK);
/* Enable SPI, Master, set clock rate fck/16  */
SPCR0 = (1<<SPE)|(1<<MSTR)|(1<<SPR1)|(1<<CPHA);

SPSR0 |= (1<<SPI2X);
}

char SPI_MasterTransmit(char cData)
{
/* Start transmission */
SPDR0 = cData;
/* Wait for transmission complete */
while(!(SPSR0 & (1<<SPIF)));

return SPDR0;
}

void init_encoder(void){
    
    DDRB |= (1<<DDB3); // digital input = 0
    
    DDRB |= (1<<CS);
    
    PORTB |= (1<<CS); // - chip select to high
    
}



uint16_t encoder_position(void) {
    uint8_t high, low;
    uint16_t full;

    // pull down encoder CS
    PORTB &= ~(1<<CS);
    _delay_us(10); // Wait >500ns
    
    // SPI transfer
    high = SPI_MasterTransmit(0x00);
    
    low = SPI_MasterTransmit(0x00);
    
    // release encoder CS back up
    PORTB |= (1<<CS); 
    
    full = (high<<8)|low;
    
    return (full>>6);
}



/*
 * 
 */
int main(int argc, char** argv) {
    
    uint16_t position, temp = 0;
    int32_t integral = 0; // accumulated position
    uint8_t i = 0; // tracking loop count for debugging purposes
    
    uart_init();
    SPI_MasterInit();
    init_encoder();
    
    while(1) {
        
        position = encoder_position();
            
        // calculate step size
        int16_t dx = position - temp;
        dx = (dx<<6)>>6; // make sure smaller angle is chosen as step size
        
        integral += dx;
        
        // ready for next run
        temp = position;
        i++;
        
        
        
        if(i == 50){
            //print status
            i = 0;
            printf("integral: %d \n", (integral*36)/102);
            printf("angle %d \n", position*36/102);
        }
        
        _delay_ms(10); 
    }
    
    
    

    return (EXIT_SUCCESS);
}
