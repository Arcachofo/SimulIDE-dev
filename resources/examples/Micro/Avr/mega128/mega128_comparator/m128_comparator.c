
#define F_CPU 16000000L

#include <avr/io.h>

void main(void)
{
    DDRC  = 0xFF;    // port c out
    DDRE  = 0x0;     // port e in
    ACSR  = 0x0;     // a-comparator setup
    SFIOR = 0x0;     // ACME=0, use AIN1

    PORTC = 0x00;

    while(1){
        if((ACSR & 0x20) != 0) // ACO = '1' (AIN0 >= AIN1)?
             PORTC= 0xFF;
        else  
             PORTC= 0x0F;
    }
}
