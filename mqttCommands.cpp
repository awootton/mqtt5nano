
#include "mqttCommands.h"

#if defined(ARDUINO)

#else

void delay(int d) {
    usleep( d * 1000);
}

#endif

namespace mqtt5nano {

    EepromItem topicStash(64, "long name","need topic");
    EepromItem tokenStash(512, "token","need_token");

    char topic[64];     // topicStash.size];
    char passWord[512]; // tokenStash.size];

    void setLongName(const char * name) {
        topicStash.write(name);
    }

}
