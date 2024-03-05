// Compiler: Mplabx_xc8_12F  Device: 12f675

#define _XTAL_FREQ 20000000UL 

#include <xc.h>
#include <pic12f675.h>

void __interrupt() myisr() // Interrupt (ISR)
{
    if( TMR1IF ){
        TMR1IF = 0;
        GPIO ^= 1<<4; // Toggle GPIO4
    }
}

void main()
{
    CMCON = 0b00000111;    // Comparator disabled
    GPIO  = 0;
    TRISIO = 0b00100000;   // GPIO5 Input
    WPU = 0;      // Clear pullups
    
    TMR1CS  = 0;  // Internal Clock
    T1CKPS0 = 1;  // Prescaler
    T1CKPS1 = 1;  //    = 8

    TMR1IE = 1;   // Enable T1 overflow Interrupt
    PEIE = 1;     // Enable periferical Ints. INTCON
    GIE  = 1;     // Enable global ints. INTCON
    
    TMR1ON = 1;   // Enable Timer1
    
    __delay_ms( 2000 );
    
    TMR1CS = 1;  // External Clock (100KHz -> 0.7629 Hz)
    
    while(1)
    {
        __delay_ms( 500 );
        GPIO ^= 1; // Toggle GPIO0
    }
}