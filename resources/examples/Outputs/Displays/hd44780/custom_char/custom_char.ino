#include <Wire.h>
#include <LiquidCrystal.h>

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  byte z;
  //Init LCD
  lcd.begin(16, 2);

  byte customChar0[8]={0x0,0x1b,0xe,0x4,0xe,0x1b,0x0,0x0}; 
  lcd.createChar(0,customChar0);  // Custom LCD Char (X -> Spento)
  byte customChar1[8]={0x0,0x1,0x3,0x16,0x1c,0x8,0x0,0x0}; 
  lcd.createChar(1,customChar1);  // Custom LCD Char (V -> Acceso)
  
  z = 0;
  lcd.setCursor(0, 1);
  //lcd.cursor();
  lcd.write(z);
  z = 1;
  lcd.setCursor(1, 1);
  lcd.write(z);
  //lcd.blink();
}

void loop() {
 
}
