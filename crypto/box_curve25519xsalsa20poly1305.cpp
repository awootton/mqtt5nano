
#include "crypto/curve25519.h"

#include "crypto/poly1305donna.h"

/*
version 20080912
D. J. Bernstein
Public domain.
*/

#include <stdint.h>

namespace nanocrypto
{
    //#define UINTPTR_MAX 4294967295UL
    //#define SODIUM_SIZE_MAX UINTPTR_MAX

#define ROUNDS 20

#define U32C(v) (v##U)

#define ROTL32(X, B) rotl32((X), (B))
    static inline uint32_t
    rotl32(const uint32_t x, const int b)
    {
        return (x << b) | (x >> (32 - b));
    }

#define LOAD32_LE(SRC) load32_le(SRC)
    static inline uint32_t
    load32_le(const uint8_t src[4])
    {
#ifdef NATIVE_LITTLE_ENDIAN
        uint32_t w;
        memcpy(&w, src, sizeof w);
        return w;
#else
        uint32_t w = (uint32_t)src[0];
        w |= (uint32_t)src[1] << 8;
        w |= (uint32_t)src[2] << 16;
        w |= (uint32_t)src[3] << 24;
        return w;
#endif
    }

#define STORE32_LE(DST, W) store32_le((DST), (W))
    static inline void
    store32_le(uint8_t dst[4], uint32_t w)
    {
#ifdef NATIVE_LITTLE_ENDIAN
        memcpy(dst, &w, sizeof w);
#else
        dst[0] = (uint8_t)w;
        w >>= 8;
        dst[1] = (uint8_t)w;
        w >>= 8;
        dst[2] = (uint8_t)w;
        w >>= 8;
        dst[3] = (uint8_t)w;
#endif
    }

#define crypto_box_curve25519xsalsa20poly1305_BEFORENMBYTES 32

    int
    crypto_core_salsa20(unsigned char *out, const unsigned char *in,
                        const unsigned char *k, const unsigned char *c);

    void
    sodium_memzero(unsigned char *const pnt, const int len)
    {
        int i = 0;
        ;
        while (i < len)
        {
            pnt[i++] = 0;
        }
    }

    static void
    crypto_core_salsa(unsigned char *out, const unsigned char *in,
                      const unsigned char *k, const unsigned char *c,
                      const int rounds)
    {
        uint32_t x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14,
            x15;
        uint32_t j0, j1, j2, j3, j4, j5, j6, j7, j8, j9, j10, j11, j12, j13, j14,
            j15;
        int i;

        j0 = x0 = 0x61707865;
        j5 = x5 = 0x3320646e;
        j10 = x10 = 0x79622d32;
        j15 = x15 = 0x6b206574;
        if (c != NULL)
        {
            j0 = x0 = LOAD32_LE(c + 0);
            j5 = x5 = LOAD32_LE(c + 4);
            j10 = x10 = LOAD32_LE(c + 8);
            j15 = x15 = LOAD32_LE(c + 12);
        }
        j1 = x1 = LOAD32_LE(k + 0);
        j2 = x2 = LOAD32_LE(k + 4);
        j3 = x3 = LOAD32_LE(k + 8);
        j4 = x4 = LOAD32_LE(k + 12);
        j11 = x11 = LOAD32_LE(k + 16);
        j12 = x12 = LOAD32_LE(k + 20);
        j13 = x13 = LOAD32_LE(k + 24);
        j14 = x14 = LOAD32_LE(k + 28);

        j6 = x6 = LOAD32_LE(in + 0);
        j7 = x7 = LOAD32_LE(in + 4);
        j8 = x8 = LOAD32_LE(in + 8);
        j9 = x9 = LOAD32_LE(in + 12);

        for (i = 0; i < rounds; i += 2)
        {
            x4 ^= ROTL32(x0 + x12, 7);
            x8 ^= ROTL32(x4 + x0, 9);
            x12 ^= ROTL32(x8 + x4, 13);
            x0 ^= ROTL32(x12 + x8, 18);
            x9 ^= ROTL32(x5 + x1, 7);
            x13 ^= ROTL32(x9 + x5, 9);
            x1 ^= ROTL32(x13 + x9, 13);
            x5 ^= ROTL32(x1 + x13, 18);
            x14 ^= ROTL32(x10 + x6, 7);
            x2 ^= ROTL32(x14 + x10, 9);
            x6 ^= ROTL32(x2 + x14, 13);
            x10 ^= ROTL32(x6 + x2, 18);
            x3 ^= ROTL32(x15 + x11, 7);
            x7 ^= ROTL32(x3 + x15, 9);
            x11 ^= ROTL32(x7 + x3, 13);
            x15 ^= ROTL32(x11 + x7, 18);
            x1 ^= ROTL32(x0 + x3, 7);
            x2 ^= ROTL32(x1 + x0, 9);
            x3 ^= ROTL32(x2 + x1, 13);
            x0 ^= ROTL32(x3 + x2, 18);
            x6 ^= ROTL32(x5 + x4, 7);
            x7 ^= ROTL32(x6 + x5, 9);
            x4 ^= ROTL32(x7 + x6, 13);
            x5 ^= ROTL32(x4 + x7, 18);
            x11 ^= ROTL32(x10 + x9, 7);
            x8 ^= ROTL32(x11 + x10, 9);
            x9 ^= ROTL32(x8 + x11, 13);
            x10 ^= ROTL32(x9 + x8, 18);
            x12 ^= ROTL32(x15 + x14, 7);
            x13 ^= ROTL32(x12 + x15, 9);
            x14 ^= ROTL32(x13 + x12, 13);
            x15 ^= ROTL32(x14 + x13, 18);
        }
        STORE32_LE(out + 0, x0 + j0);
        STORE32_LE(out + 4, x1 + j1);
        STORE32_LE(out + 8, x2 + j2);
        STORE32_LE(out + 12, x3 + j3);
        STORE32_LE(out + 16, x4 + j4);
        STORE32_LE(out + 20, x5 + j5);
        STORE32_LE(out + 24, x6 + j6);
        STORE32_LE(out + 28, x7 + j7);
        STORE32_LE(out + 32, x8 + j8);
        STORE32_LE(out + 36, x9 + j9);
        STORE32_LE(out + 40, x10 + j10);
        STORE32_LE(out + 44, x11 + j11);
        STORE32_LE(out + 48, x12 + j12);
        STORE32_LE(out + 52, x13 + j13);
        STORE32_LE(out + 56, x14 + j14);
        STORE32_LE(out + 60, x15 + j15);
    }

    int
    crypto_core_hsalsa20(unsigned char *out,
                         const unsigned char *in,
                         const unsigned char *k,
                         const unsigned char *c)
    {
        uint32_t x0, x1, x2, x3, x4, x5, x6, x7, x8,
            x9, x10, x11, x12, x13, x14, x15;
        int i;

        if (c == NULL)
        {
            x0 = U32C(0x61707865);
            x5 = U32C(0x3320646e);
            x10 = U32C(0x79622d32);
            x15 = U32C(0x6b206574);
        }
        else
        {
            x0 = LOAD32_LE(c + 0);
            x5 = LOAD32_LE(c + 4);
            x10 = LOAD32_LE(c + 8);
            x15 = LOAD32_LE(c + 12);
        }
        x1 = LOAD32_LE(k + 0);
        x2 = LOAD32_LE(k + 4);
        x3 = LOAD32_LE(k + 8);
        x4 = LOAD32_LE(k + 12);
        x11 = LOAD32_LE(k + 16);
        x12 = LOAD32_LE(k + 20);
        x13 = LOAD32_LE(k + 24);
        x14 = LOAD32_LE(k + 28);
        x6 = LOAD32_LE(in + 0);
        x7 = LOAD32_LE(in + 4);
        x8 = LOAD32_LE(in + 8);
        x9 = LOAD32_LE(in + 12);

        for (i = ROUNDS; i > 0; i -= 2)
        {
            x4 ^= ROTL32(x0 + x12, 7);
            x8 ^= ROTL32(x4 + x0, 9);
            x12 ^= ROTL32(x8 + x4, 13);
            x0 ^= ROTL32(x12 + x8, 18);
            x9 ^= ROTL32(x5 + x1, 7);
            x13 ^= ROTL32(x9 + x5, 9);
            x1 ^= ROTL32(x13 + x9, 13);
            x5 ^= ROTL32(x1 + x13, 18);
            x14 ^= ROTL32(x10 + x6, 7);
            x2 ^= ROTL32(x14 + x10, 9);
            x6 ^= ROTL32(x2 + x14, 13);
            x10 ^= ROTL32(x6 + x2, 18);
            x3 ^= ROTL32(x15 + x11, 7);
            x7 ^= ROTL32(x3 + x15, 9);
            x11 ^= ROTL32(x7 + x3, 13);
            x15 ^= ROTL32(x11 + x7, 18);
            x1 ^= ROTL32(x0 + x3, 7);
            x2 ^= ROTL32(x1 + x0, 9);
            x3 ^= ROTL32(x2 + x1, 13);
            x0 ^= ROTL32(x3 + x2, 18);
            x6 ^= ROTL32(x5 + x4, 7);
            x7 ^= ROTL32(x6 + x5, 9);
            x4 ^= ROTL32(x7 + x6, 13);
            x5 ^= ROTL32(x4 + x7, 18);
            x11 ^= ROTL32(x10 + x9, 7);
            x8 ^= ROTL32(x11 + x10, 9);
            x9 ^= ROTL32(x8 + x11, 13);
            x10 ^= ROTL32(x9 + x8, 18);
            x12 ^= ROTL32(x15 + x14, 7);
            x13 ^= ROTL32(x12 + x15, 9);
            x14 ^= ROTL32(x13 + x12, 13);
            x15 ^= ROTL32(x14 + x13, 18);
        }

        STORE32_LE(out + 0, x0);
        STORE32_LE(out + 4, x5);
        STORE32_LE(out + 8, x10);
        STORE32_LE(out + 12, x15);
        STORE32_LE(out + 16, x6);
        STORE32_LE(out + 20, x7);
        STORE32_LE(out + 24, x8);
        STORE32_LE(out + 28, x9);

        return 0;
    }

    static int
    stream_ref_xor_ic(unsigned char *c, const unsigned char *m,
                      unsigned long long mlen, const unsigned char *n, uint64_t ic,
                      const unsigned char *k)
    {
        unsigned char in[16];
        unsigned char block[64];
        unsigned char kcopy[32];
        unsigned int i;
        unsigned int u;

        if (!mlen)
        {
            return 0;
        }
        for (i = 0; i < 32; i++)
        {
            kcopy[i] = k[i];
        }
        for (i = 0; i < 8; i++)
        {
            in[i] = n[i];
        }
        for (i = 8; i < 16; i++)
        {
            in[i] = (unsigned char)(ic & 0xff);
            ic >>= 8;
        }
        while (mlen >= 64)
        {
            crypto_core_salsa20(block, in, kcopy, NULL);
            for (i = 0; i < 64; i++)
            {
                c[i] = m[i] ^ block[i];
            }
            u = 1;
            for (i = 8; i < 16; i++)
            {
                u += (unsigned int)in[i];
                in[i] = u;
                u >>= 8;
            }
            mlen -= 64;
            c += 64;
            m += 64;
        }
        if (mlen)
        {
            crypto_core_salsa20(block, in, kcopy, NULL);
            for (i = 0; i < (unsigned int)mlen; i++)
            {
                c[i] = m[i] ^ block[i];
            }
        }
        sodium_memzero(block, sizeof block);
        sodium_memzero(kcopy, sizeof kcopy);

        return 0;
    }

    // int
    // crypto_stream_xsalsa20(unsigned char *c, unsigned long long clen,
    //                        const unsigned char *n, const unsigned char *k)
    // {
    //     unsigned char subkey[32];
    //     int ret;

    //     crypto_core_hsalsa20(subkey, n, k, NULL);
    //     ret = crypto_stream_salsa20(c, clen, n + 16, subkey);
    //     sodium_memzero(subkey, sizeof subkey);

    //     return ret;
    // }

    // int
    // crypto_stream_xsalsa20_xor_ic(unsigned char *c, const unsigned char *m,
    //                               unsigned long long mlen, const unsigned char *n,
    //                               uint64_t ic, const unsigned char *k)
    // {
    //     unsigned char subkey[32];
    //     int ret;

    //     crypto_core_hsalsa20(subkey, n, k, NULL);
    //     ret = crypto_stream_salsa20_xor_ic(c, m, mlen, n + 16, ic, subkey);
    //     sodium_memzero(subkey, sizeof subkey);

    //     return ret;
    // }

    // int
    // crypto_stream_xsalsa20_xor(unsigned char *c, const unsigned char *m,
    //                            unsigned long long mlen, const unsigned char *n,
    //                            const unsigned char *k)
    // {
    //     return crypto_stream_xsalsa20_xor_ic(c, m, mlen, n, 0ULL, k);
    // }

//#define crypto_stream_xsalsa20_MESSAGEBYTES_MAX SODIUM_SIZE_MAX
#define crypto_stream_xsalsa20_MESSAGEBYTES_MAX 4294967295UL
#define crypto_stream_xsalsa20_KEYBYTES 32U
#define crypto_stream_xsalsa20_NONCEBYTES 24U

    size_t
    crypto_stream_xsalsa20_keybytes(void)
    {
        return crypto_stream_xsalsa20_KEYBYTES;
    }

    size_t
    crypto_stream_xsalsa20_noncebytes(void)
    {
        return crypto_stream_xsalsa20_NONCEBYTES;
    }

    size_t
    crypto_stream_xsalsa20_messagebytes_max(void)
    {
        return crypto_stream_xsalsa20_MESSAGEBYTES_MAX;
    }

    int
    crypto_stream_salsa20_xor_ic(unsigned char *c, const unsigned char *m,
                                 unsigned long long mlen,
                                 const unsigned char *n, uint64_t ic,
                                 const unsigned char *k)
    {
        // return implementation->stream_xor_ic(c, m, mlen, n, ic, k);
        return stream_ref_xor_ic(c, m, mlen, n, ic, k);
    }

    int
    crypto_stream_xsalsa20_xor_ic(unsigned char *c, const unsigned char *m,
                                  unsigned long long mlen, const unsigned char *n,
                                  uint64_t ic, const unsigned char *k)
    {
        unsigned char subkey[32];
        int ret;

        crypto_core_hsalsa20(subkey, n, k, NULL);
        ret = crypto_stream_salsa20_xor_ic(c, m, mlen, n + 16, ic, subkey);
        sodium_memzero(subkey, sizeof subkey);

        return ret;
    }

    int
    crypto_stream_xsalsa20_xor(unsigned char *c, const unsigned char *m,
                               unsigned long long mlen, const unsigned char *n,
                               const unsigned char *k)
    {
        return crypto_stream_xsalsa20_xor_ic(c, m, mlen, n, 0ULL, k);
    }

    int
    crypto_core_salsa20(unsigned char *out, const unsigned char *in,
                        const unsigned char *k, const unsigned char *c)
    {
        crypto_core_salsa(out, in, k, c, 20);
        return 0;
    }

    static int
    stream_ref(unsigned char *c, unsigned long long clen, const unsigned char *n,
               const unsigned char *k)
    {
        unsigned char in[16];
        unsigned char block[64];
        unsigned char kcopy[32];
        unsigned int i;
        unsigned int u;

        if (!clen)
        {
            return 0;
        }
        for (i = 0; i < 32; i++)
        {
            kcopy[i] = k[i];
        }
        for (i = 0; i < 8; i++)
        {
            in[i] = n[i];
        }
        for (i = 8; i < 16; i++)
        {
            in[i] = 0;
        }
        while (clen >= 64)
        {
            crypto_core_salsa20(c, in, kcopy, NULL);
            u = 1;
            for (i = 8; i < 16; i++)
            {
                u += (unsigned int)in[i];
                in[i] = u;
                u >>= 8;
            }
            clen -= 64;
            c += 64;
        }
        if (clen)
        {
            crypto_core_salsa20(block, in, kcopy, NULL);
            for (i = 0; i < (unsigned int)clen; i++)
            {
                c[i] = block[i];
            }
        }
        sodium_memzero(block, sizeof block);
        sodium_memzero(kcopy, sizeof kcopy);

        return 0;
    }

    int
    crypto_stream_xor(unsigned char *c, const unsigned char *m,
                      unsigned long long mlen, const unsigned char *n,
                      const unsigned char *k)
    {
        return crypto_stream_xsalsa20_xor(c, m, mlen, n, k);
    }

    int
    crypto_stream_salsa20(unsigned char *c, unsigned long long clen,
                          const unsigned char *n, const unsigned char *k)
    {
        // return implementation->stream(c, clen, n, k);
        return stream_ref(c, clen, n, k);
    }

    // void
    // crypto_stream_xsalsa20_keygen(unsigned char k[crypto_stream_xsalsa20_KEYBYTES])
    // {
    //     randombytes_buf(k, crypto_stream_xsalsa20_KEYBYTES);
    // }

    int
    crypto_stream_xsalsa20(unsigned char *c, unsigned long long clen,
                           const unsigned char *n, const unsigned char *k)
    {
        unsigned char subkey[32];
        int ret;

        crypto_core_hsalsa20(subkey, n, k, NULL);
        ret = crypto_stream_salsa20(c, clen, n + 16, subkey);
        sodium_memzero(subkey, sizeof subkey);

        return ret;
    }

    //////

    int
    crypto_secretbox_xsalsa20poly1305(unsigned char *c, const unsigned char *m,
                                      unsigned long long mlen,
                                      const unsigned char *n,
                                      const unsigned char *k)
    {
        int i;

        if (mlen < 32)
        {
            return -1;
        }
        crypto_stream_xsalsa20_xor(c, m, mlen, n, k);
        crypto_onetimeauth_poly1305(c + 16, c + 32, mlen - 32, c);
        for (i = 0; i < 16; ++i)
        {
            c[i] = 0;
        }
        return 0;
    }

    int
    crypto_secretbox_xsalsa20poly1305_open(unsigned char *m, const unsigned char *c,
                                           unsigned long long clen,
                                           const unsigned char *n,
                                           const unsigned char *k)
    {
        unsigned char subkey[32];
        int i;

        if (clen < 32)
        {
            return -1;
        }
        crypto_stream_xsalsa20(subkey, 32, n, k);
        if (crypto_onetimeauth_poly1305_verify(c + 16, c + 32,
                                               clen - 32, subkey) != 0)
        {
            return -1;
        }
        crypto_stream_xsalsa20_xor(m, c, clen, n, k);
        for (i = 0; i < 32; ++i)
        {
            m[i] = 0;
        }
        return 0;
    }

    int
    crypto_box_curve25519xsalsa20poly1305_beforenm(unsigned char *k,
                                                   const unsigned char *pk,
                                                   const unsigned char *sk)
    {
        static const unsigned char zero[16] = {0};
        unsigned char s[32];

        if (crypto_scalarmult_curve25519_ref10((unsigned char *)s, sk, pk) != 0)
        {
            return -1;
        }
        return crypto_core_hsalsa20(k, zero, s, NULL);
    }

    //////// ok //////// ok
    int
    crypto_box_curve25519xsalsa20poly1305_afternm(unsigned char *c,
                                                  const unsigned char *m,
                                                  unsigned long long mlen,
                                                  const unsigned char *n,
                                                  const unsigned char *k)
    {
        return crypto_secretbox_xsalsa20poly1305(c, m, mlen, n, k);
    }

    int
    crypto_box_curve25519xsalsa20poly1305_open_afternm(unsigned char *m,
                                                       const unsigned char *c,
                                                       unsigned long long clen,
                                                       const unsigned char *n,
                                                       const unsigned char *k)
    {
        return crypto_secretbox_xsalsa20poly1305_open(m, c, clen, n, k);
    }
    

    ////////////////

    int
    crypto_box_curve25519xsalsa20poly1305(unsigned char *c, const unsigned char *m,
                                          unsigned long long mlen,
                                          const unsigned char *n,
                                          const unsigned char *pk,
                                          const unsigned char *sk)
    {
        unsigned char k[crypto_box_curve25519xsalsa20poly1305_BEFORENMBYTES];
        int ret;

        if (crypto_box_curve25519xsalsa20poly1305_beforenm(k, pk, sk) != 0)
        {
            return -1;
        }
        ret = crypto_box_curve25519xsalsa20poly1305_afternm(c, m, mlen, n, k);
        sodium_memzero(k, sizeof k);

        return ret;
    }

    int
    crypto_box_curve25519xsalsa20poly1305_open(
        unsigned char *m, const unsigned char *c, unsigned long long clen,
        const unsigned char *n, const unsigned char *pk, const unsigned char *sk)
    {
        unsigned char k[crypto_box_curve25519xsalsa20poly1305_BEFORENMBYTES];
        int ret;

        if (crypto_box_curve25519xsalsa20poly1305_beforenm(k, pk, sk) != 0)
        {
            return -1;
        }
        ret = crypto_box_curve25519xsalsa20poly1305_open_afternm(m, c, clen, n, k);
        sodium_memzero(k, sizeof k);

        return ret;
    }


}