// Board: MightyCore:avr:32

void setup() 
{
  DDRD = 0xFF;
  PORTD = 0;
  Serial.begin(9600);
  delay( 500 );
  PORTD |= 0b00000100;
  Serial.println("This is a test");
}

void loop() {
}
