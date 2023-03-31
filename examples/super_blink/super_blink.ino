#include "mqtt5nano.h"

mqtt5nano::PackageOne one;

// An example of blink that is configurable online via the app at knotfree.net.
// FIXME: finish


void setup() {
    Serial.begin(115200);
    delay(100);
    one.setup(Serial);
}

void loop() {
    one.loop(millis(), Serial);
}
