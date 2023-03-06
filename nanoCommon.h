

#pragma once

#include  <cstdint>

namespace mqtt5nano {

    extern unsigned int getRand();

    extern void getRandomString(char *cP, int len);

    extern void moreScrambled(int entropy);

    extern void moreScrambled(const char *cP);

    extern uint64_t latestNowMillis;

    // getUnixTime applies an offset to the latestNowMillis aka millis() clock to get the unix time
    // the change is saved in the EEPROM.
    extern int getUnixTime();

    // apply an appropriate offset to the millis() clock to get the unix time
    extern void setUnixTime(uint32_t unixTime);

    extern void initUnixTime(); // used only at startup.

    extern uint64_t (*getMillis)();
}

#if defined(ARDUINO)

#if defined(ESP32)
#define PROGMEM
#endif

#else

#include "mockStream.h"

#define F(a) a

#define strncpy_P strncpy

#define PROGMEM

extern void delay(int);

#define memcpy_P(a, b, c) memcpy(a, b, c)

#endif
