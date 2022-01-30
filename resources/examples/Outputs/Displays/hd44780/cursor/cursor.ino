#include <LiquidCrystal.h>

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

 
void setup()
{
  lcd.begin(16, 2);
  
  lcd.setCursor( 0, 0 );
  lcd.print("NO CURSOR");
  delay(2000);
  
  lcd.cursor();
  lcd.setCursor( 0, 1 );
  lcd.print("CURSOR");
  delay(2000);

  lcd.blink();
  delay(2000);

  lcd.setCursor( 0, 1 );
  delay(2000);

  lcd.noCursor();
} 
 
void loop()
{
}