# TinyWireM [![Build Status](https://github.com/adafruit/TinyWireM/workflows/Arduino%20Library%20CI/badge.svg)](https://github.com/adafruit/TinyWireM/actions)[![Documentation](https://github.com/adafruit/ci-arduino/blob/master/assets/doxygen_badge.svg)](http://adafruit.github.io/TinyWireM/html/index.html)

ATtiny (e.g. Adafruit Trinket, Gemma) I2C library, adapted from BroHogan's code on Arduino Playground: http://playground.arduino.cc/Code/USIi2c

Minor changes for consistency with the Arduino 1.0 Wire library (e.g. uses write() instead of send()). Buffer size slightly increased for Adafruit_LEDBackpack use.

On the Trinket boards, pin #0 is SDA (I2C data), pin #2 is SCK (I2C clock).
