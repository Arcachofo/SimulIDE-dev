
#include <RTClib.h>
#include <LiquidCrystal.h>

#define RTC_ADDR 0x68

uint8_t counter = 0;
int mode_index = 2;

Ds1307SqwPinMode modes[] = { DS1307_OFF, DS1307_ON, DS1307_SquareWave1HZ, DS1307_SquareWave4kHz, DS1307_SquareWave8kHz, DS1307_SquareWave32kHz};
String daysW[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

LiquidCrystal lcd( 12, 11, 5, 4, 3, 2 );
RTC_DS1307 RTC;

byte second, minute, hour, dayWeek, day, month, year;

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
Serial.begin(9600);
  pinMode( 13, OUTPUT );
  lcd.begin( 16, 2 );
  lcd.clear();

  if( !RTC.begin() )     error("RTC Init ERROR");
  if( !RTC.isrunning() ) RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  if( !RTC.isrunning() ) error("RTC NOT Running!");
  lcd.print("RTC Running");
  delay( 300 );
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

  GetRtc();
  DisplayDateTime();

  digitalWrite( 13, 0 );
  delay( 300 );
}

void printDec( byte dec, byte pre )
{
  if     ( pre == 1 ) lcd.print( "/" );
  else if( pre == 2 ) lcd.print( ":" );

  if( dec < 10 ) lcd.print("0");
  lcd.print( dec, DEC );
}

void DisplayDateTime ()
{
  lcd.setCursor(2, 0);
  lcd.print( daysW[dayWeek] );
  lcd.print( " " );
  printDec( day, 0 );
  printDec( month, 1 );
  printDec( year, 1 );

  lcd.setCursor( 4, 1 );
  printDec( hour, 0 );
  printDec( minute, 2 );
  printDec( second, 2 );
}

void GetRtc()
{
  DateTime now = RTC.now();
  second  = now.second();
  minute  = now.minute();
  hour    = now.hour();
  dayWeek = 2;
  day     = now.day();
  month   = now.month();
  year    = now.year();
}
