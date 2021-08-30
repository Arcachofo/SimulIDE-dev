
#include <LiquidCrystal_AIP31068_I2C.h>

// I2C address = 0x3E , 20x4 display
LiquidCrystal_AIP31068_I2C lcd( 0x3E, 20, 4 );

void setup()
{
  lcd.init();
  lcd.setCursor(3,0);
  lcd.print("Hello, world!");
  lcd.setCursor(0,1);
  lcd.print("********************");
  lcd.setCursor(0,2);
  lcd.print("*     SimulIDE     *");
  lcd.setCursor(0,3);
  lcd.print("********************");
}

void loop(){}

