#include "mqtt5nano.h"

mqtt5nano::PackageOne one;

#if defined(ESP32)
#define LED_BUILTIN 2
#endif

// An example of blink that is configurable online via the app at knotfree.net.
// FIXME: finish

void setup() {
    Serial.begin(115200);
    delay(100);
    one.setup(Serial);

    pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output
}

void loop() {
    one.loop(millis(), Serial);
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
    delay(1000);                     // Wait for a second
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
    delay(2000);
}
