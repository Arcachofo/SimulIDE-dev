
#include <Servo.h>

Servo myservo;
int pos = 0;

void setup() 
{
  myservo.attach( 9, 1000, 2000 );
  pinMode( 13, OUTPUT );
}

void loop() 
{
  for( pos=0; pos<=180; pos += 1)
  {
    delay( 10 );
    digitalWrite( 13 , 1 );
    
    myservo.write( pos );
    
    delay( 10 );
    digitalWrite( 13 , 0 );
  }
  delay( 100 );

  for( pos=180; pos>=0; pos -= 1)
{
    delay( 10 );
    digitalWrite( 13 , 1 );
    
    myservo.write( pos );
    
    delay( 10 );
    digitalWrite( 13 , 0 );
  }
}

