#include "knotfree.h"

mqtt5nano::PackageOne one;

void setup() {
  Serial.begin(115200);
  delay(10);
  one.setup(Serial);
}

void loop() {
  one.loop(millis(), Serial);
}