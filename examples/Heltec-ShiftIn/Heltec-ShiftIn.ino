#include "Arduino.h"
#include <HX711.h>

#define DATA_PIN  GPIO2
#define CLOCK_PIN GPIO3

HX711 scale = HX711();

void setup() {
  scale.begin(DATA_PIN, CLOCK_PIN);
  scale.set_scale(10000);
  scale.set_offset(0);
}

void loop() {
  float value = scale.get_units(3);
}
