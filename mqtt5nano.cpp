
#include "eepromItem.h"

//#include <cstdlib>

#include "crypto/tinymt.h"

#include "nanobase64.h"
#include "nanoCommon.h"


namespace mqtt5nano {

    long long latestNowMillis = 0;
    long long millisUnixAdjust = 0;

    int getUnixTime(){
        return (latestNowMillis + millisUnixAdjust) / 1000;
    }

    int defaultMillis(){
        return (int)latestNowMillis;
    }

    int (*getMillis)() = defaultMillis;

    tinymt::tinymt32 rng;

    unsigned int getRand() {
        int tmp = rng();
        return tmp;
    }

    // int32_t state = 97876543;
    // unsigned int getRand() {
    //        int32_t val = ((state * 1103515245U) + 12345U) & 0x7fffffff;  
    //        state = va;;
    //        return val;
    // }

    void moreScrambled(int entropy) {
        int tmp = getRand() * entropy;
        rng.seed(tmp);
    }

    void moreScrambled(const char * cP) {
       while ( *cP != 0){
            moreScrambled(*cP);
            cP ++;
       } 
    }

    void getRandomString( char * cP, int len){
        const char * b64table = base64::getTable();
        for ( int i = 0; i < len ; i ++) {
            int r = getRand() % 62;
            char c = b64table[r];
            cP[i] = c;
        }
    }

}
