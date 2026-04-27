////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////    main.c
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

#define F_CPU 16000000UL

/*--------------------Libraries---------------------------*/
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <avr/io.h>
#include <avr/sleep.h>


// CHANGE THE INCLUDES!!! for a new laptop


/* Scheduler include files. */
#include "AVR_FreeRTOS/FreeRTOS.h"
#include "AVR_FreeRTOS/task.h"
#include "AVR_FreeRTOS/queue.h"
#include "AVR_FreeRTOS/semphr.h"

/* util stuff */
#include "lib/uart.h"
#include "time.h"
#include "util/delay.h"



/* LCD stuff */
#include "lib2/ST7735.h"
#include "lib2/LCD_GFX.h"
#include "lib2/ASCII_LUT.h"


#define CS_ENC PD7

/*---global defines---*/

volatile int16_t shared_integral = 0;
SemaphoreHandle_t SPImutex;



/*----- HELPER Functions ------*/
void Initialise()
{
    
    // LCD STUFF
    ADMUX |= (1<<REFS0);

    DDRD |= (1<<CS_ENC);
    ADCSRA |= (1<<ADEN);
    ADCSRA |= (1<<ADSC);
	lcd_init();
    LCD_setScreen(0xFFFF);
    uart_init();
    DDRD |=(1<<PD0)|(1<<PD1)|(1<<PD2)|(1<<PD3);
    
    
    SPImutex = xSemaphoreCreateMutex();
    
    

}

// added inline to save stack budget
void inline _2_A()  { PORTD = (PORTD & 0xF0) | 0x01; } // 0001
void inline _2_AB() { PORTD = (PORTD & 0xF0) | 0x03; } // 0011
void inline _2_B()  { PORTD = (PORTD & 0xF0) | 0x02; } // 0010
void inline _2_BC() { PORTD = (PORTD & 0xF0) | 0x06; } // 0110
void inline _2_C()  { PORTD = (PORTD & 0xF0) | 0x04; } // 0100
void inline _2_CD() { PORTD = (PORTD & 0xF0) | 0x0C; } // 1100
void inline _2_D()  { PORTD = (PORTD & 0xF0) | 0x08; } // 1000
void inline _2_DA() { PORTD = (PORTD & 0xF0) | 0x09; } // 1001



/*------ SPI_ functions yoinked from reference manual example -----  */

//void SPI_MasterInit(void)
//{
///* Set MOSI and SCK output, all others input */
//DDR_SPI = (1<<DD_MOSI)|(1<<DD_SCK);
///* Enable SPI, Master, set clock rate fck/16  */
//SPCR0 = (1<<SPE)|(1<<MSTR)|(1<<SPR1)|(1<<CPHA);
//
//SPSR0 |= (1<<SPI2X);
//}

char SPI_MasterTransmit(char cData)
{
    /* Start transmission */
    SPDR0 = cData;
    /* Wait for transmission complete */
    while(!(SPSR0 & (1<<SPIF)));

    return SPDR0;
}



uint16_t encoder_position(void) {
    uint8_t high, low;
    uint16_t full;
    if (xSemaphoreTake(SPImutex, portMAX_DELAY) == pdTRUE) {
    // pull down encoder CS
        PORTD &= ~(1<<CS_ENC);
        _delay_us(10); // Wait >500ns
    
    // SPI transfer
    
        high = SPI_MasterTransmit(0x00);

        low = SPI_MasterTransmit(0x00);
        PORTD |= (1<<CS_ENC);
        
        xSemaphoreGive(SPImutex);
    }
    
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




/* TASKS*/

void taskEncoder(void *p){
    
    
    uint16_t prevposition = encoder_position();
    uint16_t position = 0;

    
    while(1){
                   /*---ENCODER LOGIC---*/
            
        position = encoder_position();
        
        // calculate step size
        int16_t dx = position - prevposition;
        if (dx > 256) dx -= 512;    
        else if (dx < -256) dx += 512; 

        taskENTER_CRITICAL();
        shared_integral += dx;
        //clamping the integral value
        if(shared_integral > 1500) shared_integral = 1500;
        if(shared_integral < -1500) shared_integral = -1500;
        taskEXIT_CRITICAL();
        // ready for next run
        prevposition = position; 


        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void taskEncoder2LCD(void *p) {



    int dx = 5;

    int count = LCD_WIDTH/dx;
    char buffer[10];
    int prev = 60;
    
    while(1){
        
        
        LCD_drawLine(0,LCD_HEIGHT/2,LCD_WIDTH, LCD_HEIGHT/2, 0x0000);
//          LCD_drawCircle(50, 50, 20, 0x0000);
        int track = 0;
        
        for(int i = 0; i< count; i++){
            
            
            

            // integral + absolute step
                    
              
            itoa (shared_integral, buffer, 7);
            
            
            
            /*----PRINTING----*/
//  
            
            if (xSemaphoreTake(SPImutex, portMAX_DELAY) == pdTRUE) {
                
            LCD_drawLine(i*dx,(prev/50+LCD_HEIGHT/2),(i+1)*dx, (shared_integral/50+LCD_HEIGHT/2), 0x0000);
            prev = shared_integral;
            
            LCD_drawString(10, 20, buffer, 0x0000, 0xFFFF);
            
            xSemaphoreGive(SPImutex);
            
            vTaskDelay(pdMS_TO_TICKS(50));


            }
            
//            printf("hello \n");
            
            
            
            
//            printf("%d \n", encoder_val);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        
        
        if (xSemaphoreTake(SPImutex, portMAX_DELAY) == pdTRUE) {

            LCD_setScreen(0xFFFF);
            
            xSemaphoreGive(SPImutex);
        }
        
        
        vTaskDelay(pdMS_TO_TICKS(10));
  
    
    }
    
    
}


void taskSteppersmall(void *p){
  
    while(1){
        
        
        _2_AB();
        
        vTaskDelay(pdMS_TO_TICKS(10));

        _2_BC();
        
        vTaskDelay(pdMS_TO_TICKS(10));

        _2_CD();
                
        vTaskDelay(pdMS_TO_TICKS(10));

        _2_DA();
        
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
}



int main(void) {
    
    
    Initialise();

    
    
    printf("hello world \n");
    
    xTaskCreate(taskEncoder2LCD, "tA", 256, NULL, 3, NULL);
    
//    xTaskCreate(taskSteppersmall, "tB", 128, NULL, 3, NULL);
    
    xTaskCreate(taskEncoder, "tC", 90, NULL, 2, NULL);
    
    

    vTaskStartScheduler();
    while(1);
}
