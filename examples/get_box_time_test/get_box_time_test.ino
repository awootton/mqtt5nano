#include "mqtt5nano.h"

mqtt5nano::PackageOne one;

// This is an example of a mqtt5nano arduino sketch that has a command
// that 'boxes' a message with NaCl crypto. It is used to compare the time it takes between devices.
// It's about 250ms on an ESP8266 and 15ms on an ESP32. 

struct getBoxtime : Command {
    void init() override {
        name = "get box time";
        description = "how many ms to box a message";
    }
    void execute(Args args, badjson::Segment *params, drain &out) override {

        int startTime = millis();

        char adminPublicKey[32];
        char adminPrivateKey[32];

        char thingPublicKey[32];
        char thingPrivateKey[32];

        nanocrypto::getBoxKeyPairFromPassphrase("testString123", adminPublicKey, adminPrivateKey);

        nanocrypto::getBoxKeyPairFromPassphrase("myFamousOldeSaying", thingPublicKey, thingPrivateKey);

        int delta = millis() - startTime;
        out.write("setup:");
        out.writeInt(delta);
        out.write("\n");

        startTime = millis();

        const char *message = "this is my test message";
        char nonce[24];
        const char *tmp = "EhBJOkFN3CjwqBGzkSurniXj";
        for (int i = 0; i < 24; i++) {
            nonce[i] = tmp[i];
        }

        char destination[64]; // > 12 + 16
        sink encoded(destination, 64);

        bool ok = nanocrypto::box(&encoded, message, nonce, thingPublicKey, adminPrivateKey);
        if (!ok) {
            out.write("ERROR not ok");
        }

        delta = millis() - startTime;

        out.write("\n");
        char hexbuff[96];
        int n = base64::encode(encoded.base, encoded.amount(), hexbuff, 96);
        hexbuff[n] = 0;
        out.write(hexbuff);
        out.write("\n");
        out.write("boxing:");
        out.writeInt(delta);
        out.write("\n");
    }
};

getBoxtime gbt;

void setup() {
    Serial.begin(115200);
    delay(100);
    one.setup(Serial);
}

void loop() {
    one.loop(millis(), Serial);
}
