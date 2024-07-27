#include <SPI.h>

void setup()
{
  pinMode( 10, OUTPUT );      // set CS pin Output
  SPI.begin();

  digitalWrite( 10, LOW );    // set CS pin LOW
  for( int b=0; b<256; b++ )
  {
    SPI.transfer( b );
    delay( 10 ); 
  }
  digitalWrite( 10, HIGH );   // set CS pin HIGH
}

void loop()
{
}