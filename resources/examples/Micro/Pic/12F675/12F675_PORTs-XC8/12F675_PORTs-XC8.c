// Compiler: Mplabx_xc8_12F  Device: 12f675

#define _XTAL_FREQ 20000000UL 

#include <xc.h>
#include <pic12f675.h>

void __interrupt() myisr() // Interrupt (ISR)
{
    if( GPIF ){
        GPIF = 0;
        GPIO ^= 1<<4; // Toggle GPIO3
    }
}

void main()
{
    CMCON = 0b00000111;    // Comparator disabled
    GPIO  = 0;
    TRISIO = 0b00000011;   // GPIO0,1 Inputs
    
    WPU = 0;      // Clear pullups

    IOC  = 1;     // INTERRUPT-ON-CHANGE GPIO0
    GPIE = 1;     // Enable INTERRUPT-ON-CHANGE
    PEIE = 1;     // Enable periferical Ints. INTCON
    GIE  = 1;     // Enable global ints. INTCON
    
    while(1)
    {
        __delay_ms( 100 );
        WPU  ^= 1<<1; // Toggle Pullup GPIO1
        GPIO ^= 1<<2; // Toggle GPIO2
    }
}