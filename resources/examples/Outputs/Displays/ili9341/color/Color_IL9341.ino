
// Arduino Radar (for Proteus)


#include <SPI.h>
#include "Ucglib.h"

int Ymax = 240;
int Xmax = 320;



Ucglib_ILI9341_18x240x320_HWSPI ucg(/*cd=*/ 10, /*cs=*/ 8, /*reset=*/ 9);

void setup()
{
  // ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.begin(UCG_FONT_MODE_SOLID);
  ucg.setFont(ucg_font_6x10_tr);
  //ucg.clearScreen();

  ucg.setColor(255, 0, 0);
  ucg.setPrintPos(10, 20);
  ucg.print("RED (255, 0, 0) - R");
 
  ucg.setColor(0, 255, 0);
  ucg.setPrintPos(10, 40);
  ucg.print("GREEN (0, 255, 0) - G ");

  ucg.setColor(0, 0, 255);
  ucg.setPrintPos(10, 60);
  ucg.print("BLUE (0, 0, 255) - B");

  ucg.setColor(255, 255, 0);
  ucg.setPrintPos(10, 80);
  ucg.print("YELLOW (255, 255, 0)");

  
  ucg.setColor(255, 255, 255);
  ucg.setPrintPos(10, 100);
  ucg.print("WHITE (255, 255, 255)");

     

  delay(500);
}

void loop() {


}
