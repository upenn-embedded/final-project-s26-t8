/* 
 * File:   main.c
 * Author: sebastian
 *
 * Created on April 21, 2026, 4:50 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include "time.h"
#include "ASCII_LUT.h"
#include "LCD_GFX.h"
#include "ST7735.h"
#include "uart.h"
#include <avr/interrupt.h>

#define F_CPU 16000000UL

#include "time.h"
#include "util/delay.h"

/*
 * 
 */



#define CS_ENC PD7

/*---global variables---*/

int16_t shared_integral = 0;
uint16_t shared_position = 0;
uint16_t global_ADC = 512;

/*---- init functions ----*/

//
void init_pwm_big_step (void){

    TCCR3B |= (1<<CS32)|(1<<WGM33);//WGM 3 2 and all CSx
    TCCR3A |= (1<<COM3B1)|(1<<WGM30); //COM3Ax COM3Bx WGM 1 0
    DDRD |= (1<<PD2);
    DDRC |= (1<<PC5);
    PORTD |= (1<<PD2);
}

void init_pwm_little_step (void){
//    // I want to do OC2B and OC2A as top (mode 5) 1 0 1 or ocr2a is top and toggle on compare match?
    TCCR2B |=(1<<CS22)|(1<<WGM22); 
    TCCR2A |=(1<<COM2B0);
    DDRD |= (1<<PD3); 
    OCR2A = 100;
}

void init_interrupt_encoder(void){
    TCCR2B |= (1<<CS22)|(1<<CS21)|(1<<CS20); // prescaler of 1024
    TCCR2A |= (1<<WGM21); // CTC mode
    TIMSK2 |= (1<<OCIE2A);
}

void init_LCD(void){
    
	lcd_init();
    LCD_setScreen(0xFFFF);
    LCD_drawLine(0,LCD_HEIGHT/2,LCD_WIDTH, LCD_HEIGHT/2, 0x0000);

}

void init_encoder(void){
    
    DDRD |= (1<<CS_ENC);
    SPCR1 = (1<<SPE1) | (1<<MSTR1) | (1<<SPR11);		//Enable SPI, Master, set clock rate much slower than initially set
	SPSR1 = (1<<SPI2X1);	
    DDRC |= (1<<PC1); // set clock as output
}

void init_slider(void){
    ADMUX |= (1<<MUX1)|(1<<REFS0);
    ADCSRA |= (1<<ADEN)|(1<<ADSC)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADATE);
}

/*---- helper functions ----*/

char SPI_MasterTransmit(char cData)
{
    /* Start transmission */
    SPDR1 = cData;
    /* Wait for transmission complete */
    while(!(SPSR1 & (1<<SPIF1)));

    return SPDR1;
}

uint16_t encoder_position(void) {
    uint8_t high, low;
    uint16_t full;

// pull down encoder CS
    PORTD &= ~(1<<CS_ENC);
    _delay_us(10); // Wait >500ns

// SPI transfer

    high = SPI_MasterTransmit(0x00);

    low = SPI_MasterTransmit(0x00);
    PORTD |= (1<<CS_ENC);


// release encoder CS back up

    
    full = (high<<8)|low;
    
    return (full>>6);
}

void itoa (int16_t num, char* string, int length){
    if(num<0){
        *string = '+';
        num = -num;
    }
    else *string = '-';
    
    for(int i = 0; i< length - 1; i++){
        int temp = num/10;
        temp = num - temp*10;
        
        *(string + length - 1 - i) = temp + 48;
        num/= 10;
    }
    *(string + length) = '\0';
}


uint16_t integral_calc_and_return_position(int16_t* integral, uint16_t prevposition){
    
       
    uint16_t position = encoder_position();
    
    int16_t limit = 3000;

    // calculate step size
    int16_t dx = position - prevposition;
    if (dx > 256) dx -= 512;    
    else if (dx < -256) dx += 512; 


    *integral += dx;
    //clamping the integral value
    if(*integral > limit) *integral = limit;
    if(*integral < -limit) *integral = -limit;
    // ready for next run
    return position;
    
}



int main(int argc, char** argv) {
    
    uart_init();
    init_pwm_big_step();
    init_pwm_little_step();
    init_LCD();
    init_encoder();
    init_interrupt_encoder();
    init_slider();
    
    
    // registers for big stepper
    OCR3A = 24;
    OCR3B = 12;
    
    
    //  registers for lil stepper
    OCR2A = 255;
    OCR2B = 0;
    
    /*--- variables for LCD ---*/
    int dx = 2; // step size of graphed integral
    uint8_t factor = 50;
    int count = LCD_WIDTH/dx; // number of steps required
    char buffer[10]; // buffer to keep track of integral value
    int prev = 60; //last value of integral
    shared_integral = 0; // current value of integral
    shared_position = encoder_position();
    
    sei();
    while(1){
        for(uint8_t i = 0; i< count; i++){
            
            
            // LCD transmission code
            itoa (shared_integral, buffer, 7);
            
            
            // perhaps have an array here to track and more easily erase, like an oscilloscope TODO
            int16_t temp = shared_integral;
            LCD_drawLine(i*dx,(prev/factor+LCD_HEIGHT/2),(i+1)*dx, (shared_integral/factor+LCD_HEIGHT/2), 0x0000);
            prev = temp;
            LCD_drawString(10, 20, buffer, 0x0000, 0xFFFF);
            
            
            _delay_ms(100);
            
            
        }
        // set LCD white? or have an array of the last few integrals and just always erase those? smarter and 
            LCD_setScreen(0xFFFF);
            LCD_drawLine(0,LCD_HEIGHT/2,LCD_WIDTH, LCD_HEIGHT/2, 0x0000);


    }
    return (EXIT_SUCCESS);
}



ISR(TIMER2_COMPA_vect, ISR_BLOCK){
    shared_position = integral_calc_and_return_position(&shared_integral, shared_position);
    if(global_ADC != ADC){
        global_ADC = ADC;
        if(global_ADC<488){
            //direction change
           
            PORTC |=(1<<PC5);
            OCR3B = global_ADC/10 + 10;
            OCR3A = OCR3B*2;
            
        }
        else if(global_ADC>536){
          
            PORTC &= ~(1<<PC5);
            OCR3B = (1024 - global_ADC)/10 + 10;
            OCR3A = OCR3B*2;
        }
        else{
            OCR3B = 0;
        }
    }
}
