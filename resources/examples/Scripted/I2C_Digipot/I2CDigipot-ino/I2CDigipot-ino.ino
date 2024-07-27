#include <Wire.h>    
 
#define i2caddr 80    //Address of 24LC256 eeprom chip
 
void setup(void)
{
    //Serial.begin( 9600 );
    Wire.begin();  
    
    DDRD = 0xFF;
    
    for( int b=0; b<256; b++ )
    {
        PORTD = b;
        writeEEPROM( b );
    }
}
 
void loop()
{
    
}
 
void writeEEPROM( byte data )
{
  Wire.beginTransmission( i2caddr );
  Wire.write( data );
  Wire.endTransmission();
 
  delay( 10 );
}
