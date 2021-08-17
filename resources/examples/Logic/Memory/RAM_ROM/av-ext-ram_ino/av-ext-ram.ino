
#define CS_0 PORTB &= ~(1 << 4);
#define CS_1 PORTB |=  (1 << 4);
#define WE_0 PORTB &= ~(1 << 5);
#define WE_1 PORTB |=  (1 << 5);

void setup()
{
    DDRC = 255;
    DDRB = 255;
    DDRD = 255;
    
    // Write to RAM
    WE_0;
    for( int i=0; i<10; i++ )
    {
        CS_1;
        PORTC = i; // Address
        PORTD = i+1;
        CS_0; // Clock
        delay( 500 );
    }
    
    // Read from RAM
    DDRD = 0;
    WE_1;
    for( int i=0; i<10; i++ )
    {
        CS_1;
        PORTC = i; // Address
        CS_0; // Clock
        PORTB = PORTB & 0xF0;
        PORTB += PIND & 0x0F;
        delay( 500 );
    }
}

void loop()
{
    
}