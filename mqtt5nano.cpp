
#include "eepromItem.h"

//#include <cstdlib>

#include "crypto/tinymt.h"

#include "nanobase64.h"
#include "nanoCommon.h"
#include "eepromItem.h"


namespace mqtt5nano {

    uint64_t latestNowMillis = 0; // in millis
    uint64_t millisUnixAdjust = 1677999309 * (uint64_t)1000;// in millis

    EepromItem millisUnixAdjustStash( 12, "unix time" , "1678119117" );

    int getUnixTime(){ // in seconds
        return (latestNowMillis + millisUnixAdjust) / 1000;
    }

    // in seconds
    void setUnixTime( uint32_t unixTime) {

        serialDestination.write("setUnixTime: ");
        serialDestination.writeInt64(unixTime);
        serialDestination.writeByte('\n');

        uint64_t newAdjust = (uint64_t)unixTime * 1000 - latestNowMillis;
        uint32_t diff = newAdjust - millisUnixAdjust/1000;
        if ( diff > 10 ) { // only save if the change is more than 10 sec
            millisUnixAdjust = newAdjust;
            char buff[32];
            ByteDestination bc(buff, 32);
            bc.writeInt(millisUnixAdjust/1000);
            millisUnixAdjustStash.write(bc.buffer);
        }
    }

    uint64_t defaultMillis(){
        return latestNowMillis;
    }

    void initUnixTime(){
        char buff[32];
        ByteDestination bc(buff, 32);
        millisUnixAdjustStash.read(bc);
        slice s(bc.buffer);
        millisUnixAdjust = s.toLong();
        millisUnixAdjust = millisUnixAdjust * 1000;
        serialDestination.write("millisUnixAdjust: ");
        serialDestination.writeInt64( millisUnixAdjust);
        serialDestination.writeByte('\n');
    }

    uint64_t (*getMillis)() = defaultMillis;

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
