/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */



#ifndef UART_H
#define UART_H

#include <stdio.h>

/***************************************/
/* USER CONFIG */
/***************************************/

/**
 * Line termination type that your terminal emulator follows
 *      \r  :   #define CR
 *      \n  :   #define LF
 *      \r\n:   #define CRLF
 * For example, PuTTY and MobaXTerm end a line with only \r (CR)
 * Arduino IDE supports termination with \r\n (CRLF)
 * VSCode serial monitor extension supports all three.
 * 
 * Call determine_line_ending() in your code to see which one your terminal supports.
 */
#define CRLF

/**
 * Maximum length of input string
 */
#define MAX_STRING_LENGTH   100

/**
 * Set UART baud rate
 * This has been tested to work fine with 9600
 * 
 * If using other baud rates, slight adjustments may need to be made
 * to the UART_BAUD_PRESCALER macro (add or subtract a few counts)
 * due to baud rate error
 */
#define UART_BAUD_RATE      9600

/***************************************/
/* MACROS AND FUNCTION DECLARATIONS */
/***************************************/

#ifndef F_CPU
    #warning "F_CPU not defined. Defaulting to 16MHz"
    #define F_CPU 16000000UL
#endif

#define UART_BAUD_PRESCALER (((F_CPU / (UART_BAUD_RATE * 16UL))) - 1)

void uart_init(void);

int uart_send(char data, FILE* stream);

int uart_receive(FILE* stream);

void uart_scanf(const char* format, ...);

void determine_line_ending(void);

#endif // UART_H