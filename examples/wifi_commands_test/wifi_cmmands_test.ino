
#include <knotfree.h>



char readbuffer[1024];


mqtt5nano::streamReader reader(readbuffer, sizeof(readbuffer));

struct test1Struct : mqtt5nano::Command
{
    void init() override
    {
        SetName("get test1");
        SetDescription("output hello ignore the args");
    }
    void execute(badjson::Segment *words, drain &out) override
    {
        out.write("hello test1 execute");

    }
};
test1Struct cmd1;


void setup()
{
    Serial.begin(115200);
    delay(10);
    Serial.println("hello serial buffering test");
    // open a console and type something.
}

void loop()
{
    //serialReader.loop(Serial);
}