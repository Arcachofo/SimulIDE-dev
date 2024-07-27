// Compiler: Mplabx_xc8_12F  Device: 12f675

#define _XTAL_FREQ 20000000UL 

#include <xc.h>
#include <pic12f675.h>


void __interrupt() myisr() // Interrupt (ISR)
{
    if( CMIF ){
        CMIF = 0;
        GPIO ^= 1<<4; // Toggle GPIO4
    }
}

void main()
{
    
    GPIO  = 0;
    TRISIO = 0b00100011;   // GPIO0,1,5 Inputs
    ANSEL  = 0b00000000;   // Disable Analog
    
    CMCON = 0b00000011;    // Comparator
    
    CMIE = 1;     // Enable Comp Interrupts
    PEIE = 1;     // Enable periferical Ints. INTCON
    GIE  = 1;     // Enable global ints. INTCON
    
    while(1)
    {
        if( CINV != GPIO5 ) CINV = GPIO5;
        if( CIS != GPIO3 )  CIS  = GPIO3;
        
        __delay_ms(500);
        
        //GPIO ^= 1<<4; // Toggle GPIO5
    }
}