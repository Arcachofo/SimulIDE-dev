// Compiler: Mplabx_xc8_16Fe  Device: 16f1826

#define _XTAL_FREQ 20000000UL 

#include <xc.h>
#include <pic16f1826.h>

void __interrupt() myisr() // Interrupt (ISR)
{
    if( TMR0IF ){
        TMR0IF = 0;
        PORTB ^= 1<<4; // Toggle PORTB4
    }
}

void main()
{
    ANSELA = 0x00;
    ANSELB = 0x00;

    PORTB = 0;
    TRISB = 0b00000000;
    //WPU = 0;      // Clear pullups
    
    OPTION_REG = 0b00000010; // Prescaler = 8, 1.2207 KHz

    TMR0IE = 1;   // Enable T0 overflow Interrupt
    PEIE = 1;     // Enable periferical Ints. INTCON
    GIE  = 1;     // Enable global ints. INTCON
    
    __delay_ms( 3000 );
    
    T0CS = 1;  // External Clock
    
    while(1)
    {
        __delay_ms( 500 );
        PORTB ^= 1<<5; // Toggle PORTB5
    }
}
