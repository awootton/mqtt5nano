
#include "stdint.h"

#include <iostream>
#include <cstdlib>

#include <random>

#include "crypto.h"

#include "slices.h"

using namespace std;

namespace knotcrypto
{

#define crypto_box_SECRETKEYBYTES 32
#define crypto_box_PUBLICKEYBYTES 32
#define crypto_box_NONCEBYTES 24

    static unsigned char alicesk[crypto_box_SECRETKEYBYTES];
    static unsigned char alicepk[crypto_box_PUBLICKEYBYTES];
    static unsigned char bobsk[crypto_box_SECRETKEYBYTES];
    static unsigned char bobpk[crypto_box_PUBLICKEYBYTES];
    static unsigned char n[crypto_box_NONCEBYTES];

#define sodium_malloc malloc
#define sodium_free free

#define crypto_box_ZEROBYTES 32

#define size_t int

    int
    crypto_box_curve25519xsalsa20poly1305(unsigned char *c, const unsigned char *m,
                                          unsigned long long mlen,
                                          const unsigned char *n,
                                          const unsigned char *pk,
                                          const unsigned char *sk);

    int
    crypto_box_curve25519xsalsa20poly1305_open(
        unsigned char *m, const unsigned char *c, unsigned long long clen,
        const unsigned char *n, const unsigned char *pk, const unsigned char *sk);

    void
    randombytes_buf(void *const buf, const size_t size)
    {
        for (int i = 0; i < size; i++)
        {
            ((char *)buf)[i] = std::rand();
        }
    }

    int
    crypto_box_curve25519xsalsa20poly1305_keypair(unsigned char *pk,
                                                  unsigned char *sk)
    {
        randombytes_buf(sk, 32);

        return crypto_scalarmult_curve25519_ref10_base(pk, sk);

        // return crypto_scalarmult_curve25519_base(pk, sk);
    }

    int
    crypto_box_keypair(unsigned char *pk, unsigned char *sk)
    {
        return crypto_box_curve25519xsalsa20poly1305_keypair(pk, sk);
    }

    int
    crypto_box(unsigned char *c, const unsigned char *m,
               unsigned long long mlen, const unsigned char *n,
               const unsigned char *pk, const unsigned char *sk)
    {
        return crypto_box_curve25519xsalsa20poly1305(c, m, mlen, n, pk, sk);
    }

    int
    crypto_box_open(unsigned char *m, const unsigned char *c,
                    unsigned long long clen, const unsigned char *n,
                    const unsigned char *pk, const unsigned char *sk)
    {
        return crypto_box_curve25519xsalsa20poly1305_open(m, c, clen, n, pk, sk);
    }

    int
    main(void)
    {
        unsigned char *m;
        unsigned char *c;
        unsigned char *m2;
        size_t mlen;
        size_t mlen_max = 1000;
        size_t i;
        int ret;

        m = (unsigned char *)sodium_malloc(mlen_max);
        c = (unsigned char *)sodium_malloc(mlen_max);
        m2 = (unsigned char *)sodium_malloc(mlen_max);
        memset(m, 0, crypto_box_ZEROBYTES); // 32
        crypto_box_keypair(alicepk, alicesk);
        crypto_box_keypair(bobpk, bobsk);

        // getBoxKeyPairFromPassphrase("testString123", (char*)alicepk, (char*)alicesk);

        // getBoxKeyPairFromPassphrase("myFamousOldeSaying", (char*)bobpk, (char*)bobsk);


        int startLen = 0;
        //startLen = 23;
        for (mlen = startLen; mlen + crypto_box_ZEROBYTES <= mlen_max; mlen++)
        {
            randombytes_buf(n, crypto_box_NONCEBYTES);
            // const char *tmp = "EhBJOkFN3CjwqBGzkSurniXj";
            // for (int i = 0; i < 24; i++)
            // {
            //     n[i] = tmp[i];
            // }
            randombytes_buf(m + crypto_box_ZEROBYTES, mlen);// note that the first 32 bytes are zero and the message starts after.
            // const char * msg = "this is my test message";
            // for (int i = 0; i < strlen(msg); i++)
            // {
            //     m[i + crypto_box_ZEROBYTES] = msg[i];
            // }
            ret = crypto_box(c, m, mlen + crypto_box_ZEROBYTES, n, bobpk, alicesk);
            assert(ret == 0);
            // for ( int i = 0; i < 23 + 32 + 11; i ++ ){
            //     cout << c[i];
            // }
            // cout << "\n"; // for test message
            // need x00x00x00x00x00x00x00x00x00x00x00x00x00x00x00x00Tx8cB~xca2`xd0&x01=xeexcfx82x7fL)1/(kx00xe7hx8ebx02mEx1axc2Exd6@xd2xaa!?xcex00x00x00x00x00x00x00x00x00x00x00
            // got  x00x00x00x00x00x00x00x00x00x00x00x00x00x00x00x00Tx8cB~xca2`xd0&x01=xeexcfx82x7fL)1/(kx00xe7hx8ebx02mEx1axc2Exd6@xd2xaa!?xcex00x00x00x00x00x00x00x00x10Y

            if (crypto_box_open(m2, c, mlen + crypto_box_ZEROBYTES, n, alicepk,
                                bobsk) == 0)
            {
                for (i = 0; i < mlen + crypto_box_ZEROBYTES; ++i)
                {
                    if (m2[i] != m[i])
                    {
                        printf("bad decryption\n");
                        break;
                    }
                }
            }
            else
            {
                printf("ciphertext fails verification\n");
            }
            break;
        }
        sodium_free(m);
        sodium_free(c);
        sodium_free(m2);

        cout << "done\n";

        return 0;
    }

}

int main(void)
{
    knotcrypto::main();
}
