#include <Wire.h>    
 
#define i2caddr 80    //Address of 24LC256 eeprom chip
 
void setup(void)
{
    Serial.begin( 9600 );
    Wire.begin();  
    
    writeEEPROM( i2caddr, 0x00, 0x05 );
    writeEEPROM( i2caddr, 0x01, 0x06 );
    writeEEPROM( i2caddr, 0x02, 0x07 );
    writeEEPROM( i2caddr, 0x03, 0x08 );

    readEEPROM( i2caddr, 0x00 );
}
 
void loop()
{
}
 
void writeEEPROM( int deviceaddress, unsigned int eeaddress, byte data )
{
  Wire.beginTransmission( deviceaddress );
  Wire.write( eeaddress >> 8 );   // MSB
  Wire.write( eeaddress & 0xFF ); // LSB

  Wire.write( data );
  Wire.endTransmission();
 
  delay(5);
}
 
byte readEEPROM( int deviceaddress, unsigned int eeaddress )
{
  byte rdata = 0;
 
  Wire.beginTransmission(deviceaddress);
  Wire.write( eeaddress >> 8 );   // MSB
  Wire.write( eeaddress & 0xFF ); // LSB
  Wire.endTransmission();
 
  uint8_t bytes = Wire.requestFrom( deviceaddress, 4 );
  Serial.print( "bytes available:" );
  Serial.println( bytes );
  if( Wire.available() ) {
    Serial.println( Wire.read());
    Serial.println( Wire.read());
    Serial.println( Wire.read());
    Serial.println( Wire.read());
  }
}