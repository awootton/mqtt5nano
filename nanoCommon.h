

#pragma once

namespace mqtt5nano {

    unsigned int getRand();

    void getRandomString(char *cP, int len);

    void moreScrambled(int entropy);
    void moreScrambled(const char *cP);

    extern long long latestNowMillis;
    extern long long millisUnixAdjust;

    extern int getUnixTime();
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
