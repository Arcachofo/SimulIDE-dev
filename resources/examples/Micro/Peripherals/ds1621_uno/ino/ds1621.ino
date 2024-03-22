/***********************************************************************
 * 
 * Interfacing Arduino with DS1621 temperature sensor.
 * Temperature values are printed on 16x2 LCD screen.
 * This is a free software with NO WARRANTY.
 * https://simple-circuit.com/
 *
 ***********************************************************************/
#include <Wire.h>            // include Arduino Wire library (required for I2C devices)
#include <LiquidCrystal.h>   // include Arduino LCD library
 
// LCD module connections (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
 
// define DS1621 I2C slave address (1001+A2+A1+A0)
// A2, A1 & A0 connected to GND --> 1001000 = 0x48
#define DS1621_ADDRESS  0x48
 
void setup(void) {
  lcd.begin(16, 2);     // set up the LCD's number of columns and rows
  lcd.setCursor(0, 0);  // move cursor to column 0, row 0 [position (0, 0)]
  lcd.print("Temp =");
  
  Wire.begin();           // join i2c bus
  // initialize DS1621 sensor
  Wire.beginTransmission(DS1621_ADDRESS); // connect to DS1621 (send DS1621 address)
  Wire.write(0xAC);                       // send configuration register address (Access Config)
  Wire.write(0);                          // perform continuous conversion
  Wire.beginTransmission(DS1621_ADDRESS); // send repeated start condition
  Wire.write(0xEE);                       // send start temperature conversion command
  Wire.endTransmission();                 // stop transmission and release the I2C bus
}
 
// variables
char c_buffer[8], f_buffer[8];
 
// main loop
void loop() {
 
  delay(1000);    // wait a second
  // get temperature in tenths °C
  int16_t c_temp = get_temperature();
  // convert tenths °C to tenths °F
  int16_t f_temp = c_temp * 9/5 + 320 ;
 
  if(c_temp < 0) {   // if temperature < 0 °C
    c_temp = abs(c_temp);  // absolute value
    sprintf(c_buffer, "-%02u.%1u%cC", c_temp / 10, c_temp % 10, 223);
  }
  else {
    if (c_temp >= 1000)    // if temperature >= 100.0 °C
      sprintf(c_buffer, "%03u.%1u%cC", c_temp / 10, c_temp % 10, 223);
    else
      sprintf(c_buffer, " %02u.%1u%cC", c_temp / 10, c_temp % 10, 223);
  }
 
  if(f_temp < 0) {   // if temperature < 0 °F
    f_temp = abs(f_temp);  // absolute value
    sprintf(f_buffer, "-%02u.%1u%cF", f_temp / 10, f_temp % 10, 223);
  }
  else {
    if (f_temp >= 1000)    // if temperature >= 100.0 °F
      sprintf(f_buffer, "%03u.%1u%cF", f_temp / 10, f_temp % 10, 223);
    else
      sprintf(f_buffer, " %02u.%1u%cF", f_temp / 10, f_temp % 10, 223);
  }
 
  lcd.setCursor(6, 0);  // move cursor to position (6, 0)
  lcd.print(c_buffer);  // print c_buffer (temperature in °C)
  lcd.setCursor(6, 1);  // move cursor to position (6, 1)
  lcd.print(f_buffer);  // print f_buffer (temperature in °F)
 
}
 
int16_t get_temperature() {
  Wire.beginTransmission(DS1621_ADDRESS); // connect to DS1621 (send DS1621 address)
  Wire.write(0xAA);                       // read temperature command
  Wire.endTransmission(false);            // send repeated start condition
  Wire.requestFrom(DS1621_ADDRESS, 2);    // request 2 bytes from DS1621 and release I2C bus at end of reading
  uint8_t t_msb = Wire.read();            // read temperature MSB register
  uint8_t t_lsb = Wire.read();            // read temperature LSB register
 
  // calculate full temperature (raw value)
  int16_t raw_t = (int8_t)t_msb << 1 | t_lsb >> 7;
  // convert raw temperature value to tenths °C
  raw_t = raw_t * 10/2;
  return raw_t;
}
// end of code.
