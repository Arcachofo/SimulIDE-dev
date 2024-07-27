// Compiler: Avrgcc Device: atmega328pb

#define F_CPU 16000000UL // Defining the CPU Frequency

#include <avr/io.h>      // Contains all the I/O Register Macros
#include <util/delay.h>  // Generates a Blocking Delay

int main()
{
    DDRD   = (1 << 2); // OC3B is  D2
    ICR3   = 10000;
    OCR3B  =  5000;    
    TCCR3A = (1 << COM3B1) | (1 << WGM31);
    TCCR3B = (1 << WGM33) | (1 << WGM32) | (1 << CS31);
    
    DDRB   = (1 << 2); // OC1B is B2
    ICR1   = 10000;
    OCR1B  =  5000;
    TCCR1A = (1 << COM1B1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);

    DDRB |= 1;
    while (1)
    {
        PORTB ^= 1;
        _delay_ms( 500 );
    }
}