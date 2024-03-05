// Compiler: Mplabx_xc8_12F  Device: 12f675

#define _XTAL_FREQ 20000000UL 

#include <xc.h>
#include <pic12f675.h>


int num_adc;

void __interrupt() myisr() // Interrupt (ISR)
{
    if( ADIF ){
        ADIF = 0;
        num_adc = ((ADRESH << 8 ) | (ADRESL));
        GPIO ^= 1<<4; // Toggle GPIO3
    }
}

void main()
{
    num_adc = 0;
    
    CMCON = 0b00000111;    // Comparator disabled
    GPIO  = 0;
    TRISIO = 0b00100011;   // GPIO0,1,5 Inputs
    ANSEL  = 0b00110001;   // GPIO0 Analog
    //ADCS  = 0x02; // Focs/32 ADC Clock
    
                             // ADCON0
    ADCON0bits.CHS = 0x00; // AN0     ADC Channel
    ADFM  = 1;    // Right justified 
    ADON  = 1;    // Enable ADC
    VCFG  = 0;    // Vref+ 0=Vdd, 1=Verf Pin
    
    ADIF = 0;     // Clear ADC Int flag PIR1
    ADIE = 1;     // Enable ADC Interrupt PIE1
    PEIE = 1;     // Enable periferical Ints. INTCON
    GIE  = 1;     // Enable global ints. INTCON
    
    while(1)
    {
        if( ADFM != GPIO5 ) ADFM = GPIO5;
        if( VCFG != GPIO3 ) VCFG = GPIO3;
        __delay_us(20);
        
        GO_DONE = 1;      // Start conversion
        while( GO_DONE ); // Wait for conversion finished ADCON0
        
        GPIO ^= 1<<2; // Toggle GPIO2
    }
}