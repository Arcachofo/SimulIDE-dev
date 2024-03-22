
#include <RTClib.h>
#include <LiquidCrystal.h>

#define RTC_ADDR 0x68

uint8_t counter = 0;
int mode_index = 2;

Ds1307SqwPinMode modes[] = { DS1307_OFF, DS1307_ON, DS1307_SquareWave1HZ, DS1307_SquareWave4kHz, DS1307_SquareWave8kHz, DS1307_SquareWave32kHz};
String daysW[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

LiquidCrystal lcd( 12, 11, 5, 4, 3, 2 );
RTC_DS1307 RTC;

void error( String e )
{
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print( e );
  while( 1 )
  {
    digitalWrite( 13, 1 );
    delay(200);
    digitalWrite( 13, 0 );
    delay(200);
  }
}

void setup()
{
  pinMode( 13, OUTPUT );
  lcd.begin( 16, 2 );
  lcd.clear();

  if( !RTC.begin() ) error("RTC Init ERROR");
  if( !RTC.isrunning() )
  {
    lcd.setCursor(0, 0);
    lcd.print("RTC NOT Running");
    lcd.setCursor(2, 1);
    lcd.print( "Setting Time" );
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
    delay( 1000 );
  }
  if( !RTC.isrunning() ) error("RTC ERROR");

  lcd.clear();
  lcd.print("RTC Running");
  delay( 1000 );
  lcd.clear();
}

void loop()
{
  digitalWrite( 13, 1 );
  
  if( counter == 0 )
  {
    counter = 5;
    RTC.writeSqwPinMode( modes[mode_index] );
    mode_index++;
    if( mode_index > 5 ) mode_index = 2;
  }
  counter--;
  
  delay( 200 );

  DateTime now = RTC.now();

  lcd.setCursor(2, 0);
  lcd.print( daysW[now.dayOfTheWeek()] );
  lcd.print( " " );
  printDec( now.day(), 0 );
  printDec( now.month(), 1 );
  printDec( now.year()-2000, 1 );

  lcd.setCursor( 4, 1 );
  printDec( now.hour(), 0 );
  printDec( now.minute(), 2 );
  printDec( now.second(), 2 );

  digitalWrite( 13, 0 );
  delay( 300 );
}

void printDec( int dec, byte pre )
{
  if     ( pre == 1 ) lcd.print( "/" );
  else if( pre == 2 ) lcd.print( ":" );

  if( dec < 10 ) lcd.print("0");
  lcd.print( dec, DEC );
}

