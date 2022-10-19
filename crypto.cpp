

/** Libraries are from https://github.com/jedisct1/libsodium and https://github.com/floodyberry/poly1305-donna.
 * This part is copyright Alan Tracey Wootton. See license file.
 */

/** Sorry about the C++'ism.
 * It's just that it's messy to have these at the root
 * and the crypto folder is not on the build path.
 */

#include "crypto/Sha256.cpp"
#include "crypto/curve25519.cpp"
#include "crypto/poly1305donna.cpp"
#include "crypto/box_curve25519xsalsa20poly1305.cpp"

#include "crypto.h"

#include "slices.h"

#include <iostream>

using namespace mqtt5nano;
using namespace std;

namespace knotcrypto
{

    void Sha256Bytes(char dest[32], slice source)
    {
        Sha256Bytes(dest, source.base + source.start, source.size());
    }

    void Sha256Bytes(char dest[32], const char *source, int length)
    {
        knotcrypto::Sha256Class h;
        h.init();
        for (int i = 0; i < length; i++)
        {
            size_t n = h.write((uint8_t)source[i]);
        }
        uint8_t *r = h.result();
        for (int i = 0; i < 32; i++)
        {
            dest[i] = r[i];
        }
    }

    void Sha256FromString(char dest[32], const char *source)
    {
        int length = strlen(source);
        Sha256Bytes(dest, source, length);
    }

    void Curve25519Mpy(char dest[32], const char n[32], const char p[32])
    {
        unsigned char *destP = (unsigned char *)dest;
        const unsigned char *nP = (const unsigned char *)n;
        const unsigned char *pP = (const unsigned char *)p;

        crypto_scalarmult_curve25519_ref10(destP, nP, pP);
    }

    void Curve25519BaseMpy(char dest[32], const char n[32])
    {
        unsigned char *destP = (unsigned char *)dest;
        const unsigned char *srcP = (const unsigned char *)n;
        crypto_scalarmult_curve25519_ref10_base(destP, srcP);
    }

    void getBoxKeyPairFromPassphrase(const char *password, char publicKey[32], char privateKey[32])
    {
        Sha256FromString(privateKey, password);
        Curve25519BaseMpy(publicKey, privateKey);
    }

    const static int Overhead = 16;

    bool box(sink *adest, slice message, char nonce[24], char pubkey[32], char privkey[32])
    {
        // how do we know dest is big enough?
        // we don't. we'll transfer the bytes into in and it will protect it'self.
     
        // so here's the weird thing. It wants 32 zeroed bytes at the start of the message. 
        // and the result has 16 zero bytes at the front of it, in addition to gaining a 16 byte overhead. 
        // same deal for the unbox below. See box7.c in libsodium at https://github.com/jedisct1/libsodium
        const int messageBuffSize = message.size() + 32;
        char * messageBuffer = new char[messageBuffSize];
        memset(messageBuffer,0,32);
        memcpy(messageBuffer+32,message.base + message.start,message.size());

        const int resultBuffSize = message.size() + Overhead + 16;
        char * resultBuffer = new char[resultBuffSize];

        unsigned char *c = (unsigned char *)resultBuffer;
        const unsigned char *m = (const unsigned char *)messageBuffer;// (message.base + message.start);
        int mlen = messageBuffSize;
        const unsigned char *n = (const unsigned char *)&nonce[0];
        const unsigned char *pk = (const unsigned char *)&pubkey[0];
        const unsigned char *sk = (const unsigned char *)&privkey[0];

        int len =
            crypto_box_curve25519xsalsa20poly1305(c, m, mlen, n, pk, sk);

        for ( int i = 0; i < 23 + 32 + 11; i ++ ){
                cout << c[i];
            }

        
        adest->writeBytes(resultBuffer+16,message.size() + Overhead);
    
        delete [] resultBuffer;
        delete [] messageBuffer;

        return len >= 0; // ok
    }

    bool unbox(sink *adest, slice encrypted, char nonce[24], char pubkey[32], char privkey[32])
    {
        const int messageBuffSize = encrypted.size() + 32;// probably 16 too big
        char * messageBuffer = new char[messageBuffSize];
        memset(messageBuffer,0,messageBuffSize);
        memcpy(messageBuffer+16,encrypted.base + encrypted.start,encrypted.size());

        const int resultBuffSize = encrypted.size() + 16;
        char * resultBuffer = new char[resultBuffSize];

        unsigned char *c = (unsigned char *)resultBuffer;
        const unsigned char *m = (const unsigned char *)messageBuffer;// 16 leading zeros
        int mlen = messageBuffSize-Overhead; // 55 
        const unsigned char *n = (const unsigned char *)&nonce[0];
        const unsigned char *pk = (const unsigned char *)&pubkey[0];
        const unsigned char *sk = (const unsigned char *)&privkey[0];

        int len =
            crypto_box_curve25519xsalsa20poly1305_open(c, m, mlen, n, pk, sk);
        
        adest->writeBytes(resultBuffer+32,encrypted.size() - Overhead);
    
        delete [] resultBuffer;
        delete [] messageBuffer;

        return len >= 0; // ok
        return true;//ok
    }

}