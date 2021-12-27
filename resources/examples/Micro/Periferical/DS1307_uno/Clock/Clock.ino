
#include <Wire.h>
#include <LiquidCrystal.h>

#define RTC_ADDR 0x68

String daysW[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
LiquidCrystal lcd( 12, 11, 5, 4, 3, 2 );

byte second, minute, hour, dayWeek, day, month, year;

void setup()
{
  Wire.begin();
  lcd.begin( 16, 2 );
  lcd.clear();
}

void loop()
{
  delay(500);
  GetRtc();
  DisplayDateTime();
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
  //DateTime now = RTC.now();

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

byte bcdToDec( byte val ) { return((val/16*10)+(val % 16)); }

void GetRtc()
{
  Wire.beginTransmission( RTC_ADDR );
  Wire.write(0);                                       
  Wire.endTransmission();

  Wire.requestFrom( RTC_ADDR, 7 );             
  second  = bcdToDec( Wire.read() & 0x7f );
  minute  = bcdToDec( Wire.read() );
  hour    = bcdToDec( Wire.read() & 0x3f );
  dayWeek = bcdToDec( Wire.read() );
  day     = bcdToDec( Wire.read() );
  month   = bcdToDec( Wire.read() );
  year    = bcdToDec( Wire.read() );
}
