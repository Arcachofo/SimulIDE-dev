// Compiler: Mplabx_xc8_12F  Device: 12f675

#define _XTAL_FREQ 20000000UL 

#include <xc.h>
#include <pic12f675.h>

void __interrupt() myisr() // Interrupt (ISR)
{
    if( T0IF ){
        T0IF = 0;
        GPIO ^= 1<<4; // Toggle GPIO4
    }
}

void main()
{
    CMCON = 0b00000111;    // Comparator disabled
    GPIO  = 0;
    TRISIO = 0b00000100;   // GPIO2 Input
    WPU = 0;      // Clear pullups
    
    OPTION_REG = 0b00000010; // Prescaler = 8, 1.2207 KHz

    T0IE = 1;     // Enable T0 overflow Interrupt
    PEIE = 1;     // Enable periferical Ints. INTCON
    GIE  = 1;     // Enable global ints. INTCON
    
    __delay_ms( 2000 );
    
    T0CS = 1;  // External Clock
    
    while(1)
    {
        __delay_ms( 500 );
        GPIO ^= 1<<5; // Toggle GPIO5
    }
}