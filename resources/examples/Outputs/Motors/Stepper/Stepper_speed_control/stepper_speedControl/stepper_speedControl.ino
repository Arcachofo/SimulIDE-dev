
#include <Stepper.h>

const int SPR = 64;  // Steps Per Revolution of your stepper

Stepper myStepper( SPR, 8, 9, 10, 11 ); 

void setup() {}

void loop()
{
  int reading = analogRead( A0 );
  
  int speed = map( reading, 0, 1023, 0, SPR ); // Map reading (0-1023) to 0-SPR
  
  if( speed > 0 ) // set the motor speed:
  {
    myStepper.setSpeed( speed );
    myStepper.step( 1 );
  }
}


