

#pragma once

namespace mqtt5nano {

    unsigned int getRand();

    void getRandomString(char *cP, int len);

    void moreScrambled(int entropy);
    void moreScrambled(const char *cP);
}

#if defined(ARDUINO)

#else

    #define PROGMEM

    extern void delay(int);

    #define memcpy_P(a,b,c) memcpy(a,b,c)


#endif

 