// Compiler: Mplabx_xc8  Device: 16f887

#include <xc.h>
#include <pic16f887.h>

void __interrupt() myisr();

// Programa principal
void main(){
    // Configura registradores gerais
    ANSEL  = 0x00;
    ANSELH = 0x00;
    C1ON   = 0;
    C2ON   = 0;
    
    // Configura direcao e valores para entradas e saidas
    TRISB  = 0x03;
    TRISC  = 0x00;
    PORTC  = 0x01;

    // Configura OPTION_REG
    OPTION_REGbits.INTEDG = 1;

    // Configura interrupção externa
    INTCONbits.INTF = 0;
    INTCONbits.RBIF = 0;
    INTCONbits.GIE  = 1;
    INTCONbits.INTE = 1;
    INTCONbits.RBIE = 1;
    
    // Loop infinito
    while (1){
        // if (RB0==0) PORTD ^= 0b00000001;        
    }
}

// ISR
void __interrupt() myisr(){
    //PORTC = 0b00000011;
    if(INTF){
        PORTC ^= 0b00000001;
        INTCONbits.INTF = 0;
    }
    INTCONbits.RBIF = 0;
}
