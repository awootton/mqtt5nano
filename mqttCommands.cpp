
#include "mqttCommands.h"

#if defined(ARDUINO)

#else

void delay(int d) {
    usleep(d * 1000);
}

#endif

namespace mqtt5nano {

    EepromItem topicStash(64, "long name", "need topic");
    EepromItem tokenStash(512, "token", "need_token");

    char topic[64];     // topicStash.size];
    char passWord[512]; // tokenStash.size];

    void setLongName(const char *name) {
        topicStash.write(name);
    }

    void getTokenPayload(Destination &out) {
        char buffer[tokenStash.size];
        ByteDestination bd(&buffer[0], tokenStash.size);
        tokenStash.read(bd);
        slice token(bd.buffer);
        // serialDestination.print("token ",token,"\n");
        int firstPeriod = token.indexOf('.');
        token.start = firstPeriod + 1;
        int secondPeriod = token.indexOf('.');
        token.end = token.start + secondPeriod;
        // serialDestination.print("token chopped",token,"\n");
        bd.reset();
        token.b64Decode(&bd.buffer);
        out.write(bd.buffer);
    }

}
