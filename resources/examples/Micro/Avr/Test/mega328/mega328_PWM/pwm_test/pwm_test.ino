
int bright = 0 ;
int delta  = 1;

void setup()
{
  pinMode( 13, OUTPUT);
  /*pinMode( 3, OUTPUT);
  pinMode( 5, OUTPUT);
  pinMode( 6, OUTPUT);
  pinMode( 9, bright);
  pinMode( 10, OUTPUT);
  pinMode( 11, OUTPUT);*/
  analogWrite( 3, bright);
  analogWrite( 5, bright);
  analogWrite( 6, bright);
  analogWrite( 9, bright);
  analogWrite( 10, bright);
  analogWrite( 11, bright);
}

void loop()
{
  digitalWrite( 13, digitalRead( 2 ) );
  delay( 10 );

  bright += delta;
  if     ( bright == 254 ) delta = -1;
  else if( bright == 1   ) delta = 1;

  analogWrite( 3, bright);
  analogWrite( 5, bright);
  analogWrite( 6, bright);
  analogWrite( 9, bright);
  analogWrite( 10, bright);
  analogWrite( 11, bright);
}