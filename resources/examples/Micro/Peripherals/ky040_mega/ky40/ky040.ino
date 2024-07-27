#include <RotaryEncoder.h>

RotaryEncoder encoder(18, 19);

int previousPosition = 0;
int currentLed = 13;

int x = 0;
unsigned long previousInstant = 0;

void setup() {
  // put your setup code here, to run once:
  attachInterrupt(digitalPinToInterrupt(18), changed, CHANGE);
  attachInterrupt(digitalPinToInterrupt(19), changed2, CHANGE);
  Serial.begin(9600);

  for (int pin = 10; pin <= 13; pin++) {
    pinMode(pin, OUTPUT);
  }

  digitalWrite(currentLed, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  int position = encoder.getPosition();
  if (position != previousPosition) {
    if (position > previousPosition) {
      currentLed--;
      if (currentLed < 10) {
        currentLed = 13;
      }
    }
    else {
      currentLed++;
      if (currentLed > 13) {
        currentLed = 10;
      }
    }
    
    for (int pin = 10; pin <= 13; pin++) {
      if (pin == currentLed) {
        digitalWrite(pin, HIGH);
      }
      else {
        digitalWrite(pin, LOW);
      }
      
    }

    previousPosition = position;
  }
  //encoder.tick();

  unsigned long instant = millis();
  if (instant > previousInstant + 1000) {
    Serial.println(position);
    previousInstant = instant;
  }
}

void changed () {
  encoder.tick();
  x++;
}

void changed2 () {
  encoder.tick();
  x++;
}
