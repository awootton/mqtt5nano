
#include <mqtt5nano.h>

struct test1Struct : mqtt5nano::Command {
    void init() override {
        name = "get test1";
        description = "output hello ignore the args";
        permissions.Everywhere.notAllowed = true;
    }
    void execute(Args args, badjson::Segment *params, drain &out) override {
        out.write("hello test1 execute");
    }
};
test1Struct cmd1;

mqtt5nano::PackageOne nano;

void setup() {
    Serial.begin(115200);
    delay(10);
    nano.setup(Serial);
}

void loop() {
    nano.loop(millis(), Serial);
      sleep(1);

}