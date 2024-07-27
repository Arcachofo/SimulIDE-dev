// Board: MiniCore:avr:8

#include <LiquidCrystal.h>

#include "DHT.h"

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

const int DHTPin = 13;     // what digital pin we're connected to
DHT dht( DHTPin, DHTTYPE );

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd( rs, en, d4, d5, d6, d7 );

void setup()
{
   //Serial.begin( 9600 );
   //Serial.println("DHTxx test!");
   lcd.begin(16, 2);  // set up the LCD's number of columns and rows:
   delay( 100 );

   dht.begin();
}

void printAt( byte col, byte lin, char* text )
{
  lcd.setCursor( col,lin );
  lcd.print( text );
}

void loop() 
{
   delay( 500 );
   // Reading temperature or humidity takes about 250 milliseconds!
   float h = dht.readHumidity();
   float t = dht.readTemperature();
   
    if (isnan(h) || isnan(t)) {
        lcd.clear();
        printAt( 0, 0, "DHT11 Error");
      //Serial.println("Failed to read from DHT sensor!");
      //return;
   }
    else
    {
        printAt( 0, 0, "Temp:  "); lcd.print( t ); lcd.print( " C" );
        printAt( 0, 1, "Humid: "); lcd.print( h ); lcd.print( " %" );
    }

   /*Serial.print("Humidity: ");
   Serial.print(h);
   Serial.print(" %\t");
   Serial.print("Temperature: ");
   Serial.print(t);
   Serial.println(" *C ");*/
}