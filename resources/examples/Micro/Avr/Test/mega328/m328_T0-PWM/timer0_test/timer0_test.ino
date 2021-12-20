
// Timer 0

// output    OC0A   pin 12  (D6)
// output    OC0B   pin 11  (D5)

const byte timer0OutputA = 6;
const byte timer0OutputB = 5;

byte mode = 255;
  
void setup() 
{
    pinMode( timer0OutputA, OUTPUT ); 
    pinMode( timer0OutputB, OUTPUT ); 
    DDRD = 255;
    TIMSK0 = 0;  // no interrupts
}

void loop() 
{
    byte newMode = PINC & 0x0F;
    if( mode != newMode )
    {
        mode = newMode;
        setMode( mode );
    }
}

void setMode( byte mode )
{
    if( mode == 0 ) // Mode 0 Normal, Top 255
    {               //    F = 31.29 KHz   Duty 50%
        TCCR0A = 80;
        TCCR0B = 1;
        OCR0A = 150;
        OCR0B = 200;
    }
    else if( mode == 1 ) // Mode 1 PWM phase correct, Top 255)
    {                    //    F = 31.37 KHz   Duty 58.82%
        OCR0A = 150;
        OCR0B = 200;
        TCCR0A = 161;
        TCCR0B = 1;
    }
    else if( mode == 2 ) // Mode 2 CTC, Top OCRA)
    {                    //    F =  KHz   Duty %
        TCCR0A = 82;
        TCCR0B = 1;
        OCR0A = 150;
        OCR0B = 100;
    }
    else if( mode == 3 ) // Mode 3 fast PWM, Top 255)
    {                    //    F =  62.5KHz   Duty 58.98%
        TCCR0A = 163;
        TCCR0B = 1;
        OCR0A = 150;
        OCR0B = 200;
    }
    else if( mode == 5 ) // Mode 5 PWM phase correct, Top OCR0A)
    {                    //    F =  40.0KHz   Duty 58.98%
        TCCR0A = 161;
        TCCR0B = 9;
        OCR0A = 200;    // number of counts for a cycle
        OCR0B = 150;    // duty cycle within OCR0A
    }
    else if( mode == 7 ) // Mode 7 fast PWM, Top OCR0A)
    {                    //    F =  40.0KHz   Duty 58.98%
        TCCR0A = 163;
        TCCR0B = 9;
        OCR0A = 200;
        OCR0B = 150;
    }
}