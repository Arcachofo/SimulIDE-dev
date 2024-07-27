
byte data;

void setup() 
{
    DDRB = 0xFF;
    PORTB = 0;
    Serial.begin(9600);
    data = 0;
}

void loop() 
{
    Serial.write( data );
    PORTB = data;
    data++;
    delay( 500 );
}
