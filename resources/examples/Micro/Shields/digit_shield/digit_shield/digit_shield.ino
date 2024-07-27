#include <DigitShield.h>


void setup() {
  DigitShield.begin();
}

void loop() {

  int counter = 0;
  int d = 250;

  DigitShield.setLeadingZeros(true);
  for(int i=0;i<=9999;i+=1111) {
    DigitShield.setValue(i);
    delay(d);
  }

  for(int i=1;i<=4;i++) {
    DigitShield.setDigit(i, 0xF);
  }

  for(int i=1;i<=4;i++) {
    DigitShield.setDecimalPoint(i, true);
  }
  delay(d);
  for(int i=1;i<=4;i++) {
    DigitShield.setDecimalPoint(i, false);
  }

  d = 10;

  while (true) {
    DigitShield.setValue(counter);
    counter = counter + 1;
    delay(d);
    if (counter >= 10000) {
      break;
    }
  }
}

