
#include <mqtt5nano.h>

fix me this is outdated.

#define LED_BUILTIN 2 // for ESP32

    struct blinker : mqtt5nano::TimedItem { // define it
    bool isOn = false;
    void init() override {
        SetInterval(1200); // 1.2 second
    }
    void execute() override {
        if (isOn) {
            digitalWrite(LED_BUILTIN, LOW);
        } else {
            digitalWrite(LED_BUILTIN, HIGH);
        }
        isOn = !isOn;
    }
};
blinker myBlinker; // create it

void setup() {
    Serial.begin(115200);
    delay(10);
    Serial.println("hello blink test");
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    myBlinker.loop(millis()); // call it
}