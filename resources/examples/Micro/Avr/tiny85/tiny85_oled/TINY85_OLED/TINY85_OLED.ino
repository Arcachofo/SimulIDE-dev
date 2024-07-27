// TEST OLED IN SSD1306
// OLED ADDR.: 60 (3C)

#include <TinyWireM.h>
#include <Tiny4kOLED.h>

void setup() 
{
  oled.begin();
  oled.clear();
  oled.on();
  oled.switchRenderFrame();
}

void loop() 
{
  oled.clear();
  oled.setFont(FONT8X16);
  oled.setCursor(6, 0);
  oled.print(F("OLED Test"));
  oled.switchFrame();
  delay(500);
}
