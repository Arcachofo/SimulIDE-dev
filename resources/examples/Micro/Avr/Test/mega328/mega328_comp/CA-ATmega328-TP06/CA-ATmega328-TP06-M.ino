
void setup()
{
    DDRB |= (1<<PB5) | (1<<PB3);
    ADCSRB = 0;				  // ADC Control and Status Register B
    ADCSRA &= ~(1 << ADEN);         // Desabilita ADC
    ACSR = 0; //(1<<ACBG) | ~(1<<ACD);  // configura para referencia interna de 1,1V em AN0 e garante desabilitar o ADC
    DIDR1 = 0x03;                   // desabilita o modo de saida digital de AN0 e AN1
}

void loop()
{
    //if(ACSR & (1<<ACO)) PORTB &= ~(1<<PB5);
    //else                PORTB |= (1<<PB5);
    if( PORTB != ACSR )
    {
        if(ACSR & (1<<ACI)) ACSR = _BV( ACI );
        PORTB = ACSR;
    }
}
