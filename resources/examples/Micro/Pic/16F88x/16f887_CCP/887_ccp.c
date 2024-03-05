// Compiler: Mplabx_xc8  Device: 16f887

#define _XTAL_FREQ 20000000UL       // crysTAL FREQuency

// PIC16F887 Configuration Bit Settings
// CONFIG1
#pragma config FOSC  = HS           // OSCillator selection
#pragma config WDTE  = OFF          // WatchDog Time Enable
#pragma config PWRTE = OFF          // PoWeR-up Timer Enable
#pragma config MCLRE = ON           // Master CLeaR function Enable
#pragma config CP    = OFF          // Code Protection bit
#pragma config CPD   = OFF          // Data Code Protection bit
#pragma config BOREN = ON           // BrownOut REset SelectioN bit
#pragma config IESO  = ON           // Internal External SwitchOver bit
#pragma config FCMEN = ON           // Fails-safe Clock Monitor ENable
#pragma config LVP   = OFF          // Low Voltage Programming enable

// CONFIG2
#pragma config BOR4V = BOR40V       // Brown-Out Reset selection bit
#pragma config WRT   = OFF          // Flash program memory self Write enable bits

#include <xc.h>
#include <pic16f887.h>

#include "ccp00.h"    // CCP Modes

void __interrupt() myisr() // Interrupsion SubRoutine (ISR)
{
    if( CCP1IF ){        // CCP1 Interrupt
        PORTD ^= 0b00000010;
        CCP1IF = 0;
    }
    if( CCP2IF ){        // CCP2 Interrupt
        PORTD ^= 0b00000100;
        CCP2IF = 0;
    }
}

void main()
{
    ANSEL = 0;
    ANSELH = 0;
    
    TRISD = 0;
    TRISC = 0;
                          // Configure Timer1 To Operate In Timer Mode
    TMR1 = 0;             // Clear Timer1 Register. To start counting from 0
    TMR1CS = 0;           // Set clock source (timer mode)
    T1CKPS0 = 0;          // Set prescaler ratio (1:1)
    T1CKPS1 = 0;
    TMR1ON = 1;           // Start Timer 1

    CCP1CON = comTog;     // CCP1 Compare mode, toggle on match
    CCPR1H = 128;         // Match at 32768, 50% duty
    CCP1IE = 1;           // Enable CCP2 Interrupt

    CCP2CON = comSET;     // CCP2 Compare mode, set output on match
    CCPR2H = 128;         // Match at 32768, 50% duty
    CCP2IE = 1;           // Enable CCP2 Interrupt
    
    PEIE = 1;             // Enable Periferical Interrupts
    GIE = 1;              // Enable General Interrupts
    
    __delay_ms( 500 );
    
    while (1)
    {
        if( RE0 == 1 ){
            CCP1CON = PORTA;
            CCP2CON = PORTB;
        }
        __delay_ms( 100 );
    }
}

