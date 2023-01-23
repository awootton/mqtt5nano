/** This is not copyright by me. Portions are from libsodium.
 */

#include "crypto/curve25519.h"

namespace nanocrypto
{

#ifndef CRYPTO_ALIGN
#if defined(__INTEL_COMPILER) || defined(_MSC_VER)
#define CRYPTO_ALIGN(x) __declspec(align(x))
#else
#define CRYPTO_ALIGN(x) __attribute__((aligned(x)))
#endif
#endif

#define SODIUM_MIN(A, B) ((A) < (B) ? (A) : (B))
#define SODIUM_SIZE_MAX SODIUM_MIN(UINT64_MAX, SIZE_MAX)
#define COMPILER_ASSERT(X) (void)sizeof(char[(X) ? 1 : -1])

    /*
     fe means field element.
     Here the field is \Z/(2^255-19).
     */

#ifdef HAVE_TI_MODE
    typedef uint64_t fe25519[5];
#else
    typedef int32_t fe25519[10];
#endif

    typedef struct
    {
        fe25519 X;
        fe25519 Y;
        fe25519 Z;
        fe25519 T;
    } ge25519_p3;

    int
    sodium_is_zero(const unsigned char *n, const size_t nlen)
    {
        size_t i;
        volatile unsigned char d = 0U;

        for (i = 0U; i < nlen; i++)
        {
            d |= n[i];
        }
        return 1 & ((d - 1) >> 8);
    }

    void fe25519_invert(fe25519 out, const fe25519 z);
    void fe25519_frombytes(fe25519 h, const unsigned char *s);
    void fe25519_tobytes(unsigned char *s, const fe25519 h);

    static inline uint64_t
    load_3(const unsigned char *in)
    {
        uint64_t result;

        result = (uint64_t)in[0];
        result |= ((uint64_t)in[1]) << 8;
        result |= ((uint64_t)in[2]) << 16;

        return result;
    }

    static inline uint64_t
    load_4(const unsigned char *in)
    {
        uint64_t result;

        result = (uint64_t)in[0];
        result |= ((uint64_t)in[1]) << 8;
        result |= ((uint64_t)in[2]) << 16;
        result |= ((uint64_t)in[3]) << 24;

        return result;
    }

    /*
     Ignores top bit of s.
     */

    void
    fe25519_frombytes(fe25519 h, const unsigned char *s)
    {
        int64_t h0 = load_4(s);
        int64_t h1 = load_3(s + 4) << 6;
        int64_t h2 = load_3(s + 7) << 5;
        int64_t h3 = load_3(s + 10) << 3;
        int64_t h4 = load_3(s + 13) << 2;
        int64_t h5 = load_4(s + 16);
        int64_t h6 = load_3(s + 20) << 7;
        int64_t h7 = load_3(s + 23) << 5;
        int64_t h8 = load_3(s + 26) << 4;
        int64_t h9 = (load_3(s + 29) & 8388607) << 2;

        int64_t carry0;
        int64_t carry1;
        int64_t carry2;
        int64_t carry3;
        int64_t carry4;
        int64_t carry5;
        int64_t carry6;
        int64_t carry7;
        int64_t carry8;
        int64_t carry9;

        carry9 = (h9 + (int64_t)(1L << 24)) >> 25;
        h0 += carry9 * 19;
        h9 -= carry9 * ((uint64_t)1L << 25);
        carry1 = (h1 + (int64_t)(1L << 24)) >> 25;
        h2 += carry1;
        h1 -= carry1 * ((uint64_t)1L << 25);
        carry3 = (h3 + (int64_t)(1L << 24)) >> 25;
        h4 += carry3;
        h3 -= carry3 * ((uint64_t)1L << 25);
        carry5 = (h5 + (int64_t)(1L << 24)) >> 25;
        h6 += carry5;
        h5 -= carry5 * ((uint64_t)1L << 25);
        carry7 = (h7 + (int64_t)(1L << 24)) >> 25;
        h8 += carry7;
        h7 -= carry7 * ((uint64_t)1L << 25);

        carry0 = (h0 + (int64_t)(1L << 25)) >> 26;
        h1 += carry0;
        h0 -= carry0 * ((uint64_t)1L << 26);
        carry2 = (h2 + (int64_t)(1L << 25)) >> 26;
        h3 += carry2;
        h2 -= carry2 * ((uint64_t)1L << 26);
        carry4 = (h4 + (int64_t)(1L << 25)) >> 26;
        h5 += carry4;
        h4 -= carry4 * ((uint64_t)1L << 26);
        carry6 = (h6 + (int64_t)(1L << 25)) >> 26;
        h7 += carry6;
        h6 -= carry6 * ((uint64_t)1L << 26);
        carry8 = (h8 + (int64_t)(1L << 25)) >> 26;
        h9 += carry8;
        h8 -= carry8 * ((uint64_t)1L << 26);

        h[0] = (int32_t)h0;
        h[1] = (int32_t)h1;
        h[2] = (int32_t)h2;
        h[3] = (int32_t)h3;
        h[4] = (int32_t)h4;
        h[5] = (int32_t)h5;
        h[6] = (int32_t)h6;
        h[7] = (int32_t)h7;
        h[8] = (int32_t)h8;
        h[9] = (int32_t)h9;
    }

    /*
     Preconditions:
     |h| bounded by 1.1*2^26,1.1*2^25,1.1*2^26,1.1*2^25,etc.

     Write p=2^255-19; q=floor(h/p).
     Basic claim: q = floor(2^(-255)(h + 19 2^(-25)h9 + 2^(-1))).

     Proof:
     Have |h|<=p so |q|<=1 so |19^2 2^(-255) q|<1/4.
     Also have |h-2^230 h9|<2^231 so |19 2^(-255)(h-2^230 h9)|<1/4.

     Write y=2^(-1)-19^2 2^(-255)q-19 2^(-255)(h-2^230 h9).
     Then 0<y<1.

     Write r=h-pq.
     Have 0<=r<=p-1=2^255-20.
     Thus 0<=r+19(2^-255)r<r+19(2^-255)2^255<=2^255-1.

     Write x=r+19(2^-255)r+y.
     Then 0<x<2^255 so floor(2^(-255)x) = 0 so floor(q+2^(-255)x) = q.

     Have q+2^(-255)x = 2^(-255)(h + 19 2^(-25) h9 + 2^(-1))
     so floor(2^(-255)(h + 19 2^(-25) h9 + 2^(-1))) = q.
    */

    static void
    fe25519_reduce(fe25519 h, const fe25519 f)
    {
        int32_t h0 = f[0];
        int32_t h1 = f[1];
        int32_t h2 = f[2];
        int32_t h3 = f[3];
        int32_t h4 = f[4];
        int32_t h5 = f[5];
        int32_t h6 = f[6];
        int32_t h7 = f[7];
        int32_t h8 = f[8];
        int32_t h9 = f[9];

        int32_t q;
        int32_t carry0, carry1, carry2, carry3, carry4, carry5, carry6, carry7, carry8, carry9;

        q = (19 * h9 + ((uint32_t)1L << 24)) >> 25;
        q = (h0 + q) >> 26;
        q = (h1 + q) >> 25;
        q = (h2 + q) >> 26;
        q = (h3 + q) >> 25;
        q = (h4 + q) >> 26;
        q = (h5 + q) >> 25;
        q = (h6 + q) >> 26;
        q = (h7 + q) >> 25;
        q = (h8 + q) >> 26;
        q = (h9 + q) >> 25;

        /* Goal: Output h-(2^255-19)q, which is between 0 and 2^255-20. */
        h0 += 19 * q;
        /* Goal: Output h-2^255 q, which is between 0 and 2^255-20. */

        carry0 = h0 >> 26;
        h1 += carry0;
        h0 -= carry0 * ((uint32_t)1L << 26);
        carry1 = h1 >> 25;
        h2 += carry1;
        h1 -= carry1 * ((uint32_t)1L << 25);
        carry2 = h2 >> 26;
        h3 += carry2;
        h2 -= carry2 * ((uint32_t)1L << 26);
        carry3 = h3 >> 25;
        h4 += carry3;
        h3 -= carry3 * ((uint32_t)1L << 25);
        carry4 = h4 >> 26;
        h5 += carry4;
        h4 -= carry4 * ((uint32_t)1L << 26);
        carry5 = h5 >> 25;
        h6 += carry5;
        h5 -= carry5 * ((uint32_t)1L << 25);
        carry6 = h6 >> 26;
        h7 += carry6;
        h6 -= carry6 * ((uint32_t)1L << 26);
        carry7 = h7 >> 25;
        h8 += carry7;
        h7 -= carry7 * ((uint32_t)1L << 25);
        carry8 = h8 >> 26;
        h9 += carry8;
        h8 -= carry8 * ((uint32_t)1L << 26);
        carry9 = h9 >> 25;
        h9 -= carry9 * ((uint32_t)1L << 25);

        h[0] = h0;
        h[1] = h1;
        h[2] = h2;
        h[3] = h3;
        h[4] = h4;
        h[5] = h5;
        h[6] = h6;
        h[7] = h7;
        h[8] = h8;
        h[9] = h9;
    }

    /*
     Goal: Output h0+...+2^255 h10-2^255 q, which is between 0 and 2^255-20.
     Have h0+...+2^230 h9 between 0 and 2^255-1;
     evidently 2^255 h10-2^255 q = 0.

     Goal: Output h0+...+2^230 h9.
     */

    void
    fe25519_tobytes(unsigned char *s, const fe25519 h)
    {
        fe25519 t;

        fe25519_reduce(t, h);
        s[0] = t[0] >> 0;
        s[1] = t[0] >> 8;
        s[2] = t[0] >> 16;
        s[3] = (t[0] >> 24) | (t[1] * ((uint32_t)1 << 2));
        s[4] = t[1] >> 6;
        s[5] = t[1] >> 14;
        s[6] = (t[1] >> 22) | (t[2] * ((uint32_t)1 << 3));
        s[7] = t[2] >> 5;
        s[8] = t[2] >> 13;
        s[9] = (t[2] >> 21) | (t[3] * ((uint32_t)1 << 5));
        s[10] = t[3] >> 3;
        s[11] = t[3] >> 11;
        s[12] = (t[3] >> 19) | (t[4] * ((uint32_t)1 << 6));
        s[13] = t[4] >> 2;
        s[14] = t[4] >> 10;
        s[15] = t[4] >> 18;
        s[16] = t[5] >> 0;
        s[17] = t[5] >> 8;
        s[18] = t[5] >> 16;
        s[19] = (t[5] >> 24) | (t[6] * ((uint32_t)1 << 1));
        s[20] = t[6] >> 7;
        s[21] = t[6] >> 15;
        s[22] = (t[6] >> 23) | (t[7] * ((uint32_t)1 << 3));
        s[23] = t[7] >> 5;
        s[24] = t[7] >> 13;
        s[25] = (t[7] >> 21) | (t[8] * ((uint32_t)1 << 4));
        s[26] = t[8] >> 4;
        s[27] = t[8] >> 12;
        s[28] = (t[8] >> 20) | (t[9] * ((uint32_t)1 << 6));
        s[29] = t[9] >> 2;
        s[30] = t[9] >> 10;
        s[31] = t[9] >> 18;
    }

    /*
     h = 0
     */

    static inline void
    fe25519_0(fe25519 h)
    {
        memset(&h[0], 0, 10 * sizeof h[0]);
    }

    /*
     h = 1
     */

    static inline void
    fe25519_1(fe25519 h)
    {
        h[0] = 1;
        h[1] = 0;
        memset(&h[2], 0, 8 * sizeof h[0]);
    }

    /*
     h = f + g
     Can overlap h with f or g.
     *
     Preconditions:
     |f| bounded by 1.1*2^25,1.1*2^24,1.1*2^25,1.1*2^24,etc.
     |g| bounded by 1.1*2^25,1.1*2^24,1.1*2^25,1.1*2^24,etc.
     *
     Postconditions:
     |h| bounded by 1.1*2^26,1.1*2^25,1.1*2^26,1.1*2^25,etc.
     */

    static inline void
    fe25519_add(fe25519 h, const fe25519 f, const fe25519 g)
    {
        int32_t h0 = f[0] + g[0];
        int32_t h1 = f[1] + g[1];
        int32_t h2 = f[2] + g[2];
        int32_t h3 = f[3] + g[3];
        int32_t h4 = f[4] + g[4];
        int32_t h5 = f[5] + g[5];
        int32_t h6 = f[6] + g[6];
        int32_t h7 = f[7] + g[7];
        int32_t h8 = f[8] + g[8];
        int32_t h9 = f[9] + g[9];

        h[0] = h0;
        h[1] = h1;
        h[2] = h2;
        h[3] = h3;
        h[4] = h4;
        h[5] = h5;
        h[6] = h6;
        h[7] = h7;
        h[8] = h8;
        h[9] = h9;
    }

    /*
     h = f - g
     Can overlap h with f or g.
     *
     Preconditions:
     |f| bounded by 1.1*2^25,1.1*2^24,1.1*2^25,1.1*2^24,etc.
     |g| bounded by 1.1*2^25,1.1*2^24,1.1*2^25,1.1*2^24,etc.
     *
     Postconditions:
     |h| bounded by 1.1*2^26,1.1*2^25,1.1*2^26,1.1*2^25,etc.
     */

    static void
    fe25519_sub(fe25519 h, const fe25519 f, const fe25519 g)
    {
        int32_t h0 = f[0] - g[0];
        int32_t h1 = f[1] - g[1];
        int32_t h2 = f[2] - g[2];
        int32_t h3 = f[3] - g[3];
        int32_t h4 = f[4] - g[4];
        int32_t h5 = f[5] - g[5];
        int32_t h6 = f[6] - g[6];
        int32_t h7 = f[7] - g[7];
        int32_t h8 = f[8] - g[8];
        int32_t h9 = f[9] - g[9];

        h[0] = h0;
        h[1] = h1;
        h[2] = h2;
        h[3] = h3;
        h[4] = h4;
        h[5] = h5;
        h[6] = h6;
        h[7] = h7;
        h[8] = h8;
        h[9] = h9;
    }

    /*
     h = -f
     *
     Preconditions:
     |f| bounded by 1.1*2^25,1.1*2^24,1.1*2^25,1.1*2^24,etc.
     *
     Postconditions:
     |h| bounded by 1.1*2^25,1.1*2^24,1.1*2^25,1.1*2^24,etc.
     */

    static inline void
    fe25519_neg(fe25519 h, const fe25519 f)
    {
        int32_t h0 = -f[0];
        int32_t h1 = -f[1];
        int32_t h2 = -f[2];
        int32_t h3 = -f[3];
        int32_t h4 = -f[4];
        int32_t h5 = -f[5];
        int32_t h6 = -f[6];
        int32_t h7 = -f[7];
        int32_t h8 = -f[8];
        int32_t h9 = -f[9];

        h[0] = h0;
        h[1] = h1;
        h[2] = h2;
        h[3] = h3;
        h[4] = h4;
        h[5] = h5;
        h[6] = h6;
        h[7] = h7;
        h[8] = h8;
        h[9] = h9;
    }

    /*
     Replace (f,g) with (g,g) if b == 1;
     replace (f,g) with (f,g) if b == 0.
     *
     Preconditions: b in {0,1}.
     */

    static void
    fe25519_cmov(fe25519 f, const fe25519 g, unsigned int b)
    {
        uint32_t mask = (uint32_t)(-(int32_t)b);
        int32_t f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;
        int32_t x0, x1, x2, x3, x4, x5, x6, x7, x8, x9;

        f0 = f[0];
        f1 = f[1];
        f2 = f[2];
        f3 = f[3];
        f4 = f[4];
        f5 = f[5];
        f6 = f[6];
        f7 = f[7];
        f8 = f[8];
        f9 = f[9];

        x0 = f0 ^ g[0];
        x1 = f1 ^ g[1];
        x2 = f2 ^ g[2];
        x3 = f3 ^ g[3];
        x4 = f4 ^ g[4];
        x5 = f5 ^ g[5];
        x6 = f6 ^ g[6];
        x7 = f7 ^ g[7];
        x8 = f8 ^ g[8];
        x9 = f9 ^ g[9];

#ifdef HAVE_INLINE_ASM
        __asm__ __volatile__(""
                             : "+r"(mask));
#endif

        x0 &= mask;
        x1 &= mask;
        x2 &= mask;
        x3 &= mask;
        x4 &= mask;
        x5 &= mask;
        x6 &= mask;
        x7 &= mask;
        x8 &= mask;
        x9 &= mask;

        f[0] = f0 ^ x0;
        f[1] = f1 ^ x1;
        f[2] = f2 ^ x2;
        f[3] = f3 ^ x3;
        f[4] = f4 ^ x4;
        f[5] = f5 ^ x5;
        f[6] = f6 ^ x6;
        f[7] = f7 ^ x7;
        f[8] = f8 ^ x8;
        f[9] = f9 ^ x9;
    }

    static void
    fe25519_cswap(fe25519 f, fe25519 g, unsigned int b)
    {
        uint32_t mask = (uint32_t)(-(int64_t)b);
        int32_t f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;
        int32_t g0, g1, g2, g3, g4, g5, g6, g7, g8, g9;
        int32_t x0, x1, x2, x3, x4, x5, x6, x7, x8, x9;

        f0 = f[0];
        f1 = f[1];
        f2 = f[2];
        f3 = f[3];
        f4 = f[4];
        f5 = f[5];
        f6 = f[6];
        f7 = f[7];
        f8 = f[8];
        f9 = f[9];

        g0 = g[0];
        g1 = g[1];
        g2 = g[2];
        g3 = g[3];
        g4 = g[4];
        g5 = g[5];
        g6 = g[6];
        g7 = g[7];
        g8 = g[8];
        g9 = g[9];

        x0 = f0 ^ g0;
        x1 = f1 ^ g1;
        x2 = f2 ^ g2;
        x3 = f3 ^ g3;
        x4 = f4 ^ g4;
        x5 = f5 ^ g5;
        x6 = f6 ^ g6;
        x7 = f7 ^ g7;
        x8 = f8 ^ g8;
        x9 = f9 ^ g9;

#ifdef HAVE_INLINE_ASM
        __asm__ __volatile__(""
                             : "+r"(mask));
#endif

        x0 &= mask;
        x1 &= mask;
        x2 &= mask;
        x3 &= mask;
        x4 &= mask;
        x5 &= mask;
        x6 &= mask;
        x7 &= mask;
        x8 &= mask;
        x9 &= mask;

        f[0] = f0 ^ x0;
        f[1] = f1 ^ x1;
        f[2] = f2 ^ x2;
        f[3] = f3 ^ x3;
        f[4] = f4 ^ x4;
        f[5] = f5 ^ x5;
        f[6] = f6 ^ x6;
        f[7] = f7 ^ x7;
        f[8] = f8 ^ x8;
        f[9] = f9 ^ x9;

        g[0] = g0 ^ x0;
        g[1] = g1 ^ x1;
        g[2] = g2 ^ x2;
        g[3] = g3 ^ x3;
        g[4] = g4 ^ x4;
        g[5] = g5 ^ x5;
        g[6] = g6 ^ x6;
        g[7] = g7 ^ x7;
        g[8] = g8 ^ x8;
        g[9] = g9 ^ x9;
    }

    /*
     h = f
     */

    static inline void
    fe25519_copy(fe25519 h, const fe25519 f)
    {
        memcpy(h, f, 10 * 4);
    }

    /*
     return 1 if f is in {1,3,5,...,q-2}
     return 0 if f is in {0,2,4,...,q-1}

     Preconditions:
     |f| bounded by 1.1*2^26,1.1*2^25,1.1*2^26,1.1*2^25,etc.
     */

    static inline int
    fe25519_isnegative(const fe25519 f)
    {
        unsigned char s[32];

        fe25519_tobytes(s, f);

        return s[0] & 1;
    }

    /*
     return 1 if f == 0
     return 0 if f != 0

     Preconditions:
     |f| bounded by 1.1*2^26,1.1*2^25,1.1*2^26,1.1*2^25,etc.
     */

    static inline int
    fe25519_iszero(const fe25519 f)
    {
        unsigned char s[32];

        fe25519_tobytes(s, f);

        return sodium_is_zero(s, 32);
    }

    /*
     h = f * g
     Can overlap h with f or g.
     *
     Preconditions:
     |f| bounded by 1.65*2^26,1.65*2^25,1.65*2^26,1.65*2^25,etc.
     |g| bounded by 1.65*2^26,1.65*2^25,1.65*2^26,1.65*2^25,etc.
     *
     Postconditions:
     |h| bounded by 1.01*2^25,1.01*2^24,1.01*2^25,1.01*2^24,etc.
     */

    /*
     Notes on implementation strategy:
     *
     Using schoolbook multiplication.
     Karatsuba would save a little in some cost models.
     *
     Most multiplications by 2 and 19 are 32-bit precomputations;
     cheaper than 64-bit postcomputations.
     *
     There is one remaining multiplication by 19 in the carry chain;
     one *19 precomputation can be merged into this,
     but the resulting data flow is considerably less clean.
     *
     There are 12 carries below.
     10 of them are 2-way parallelizable and vectorizable.
     Can get away with 11 carries, but then data flow is much deeper.
     *
     With tighter constraints on inputs can squeeze carries into int32.
     */

    static void
    fe25519_mul(fe25519 h, const fe25519 f, const fe25519 g)
    {
        int32_t f0 = f[0];
        int32_t f1 = f[1];
        int32_t f2 = f[2];
        int32_t f3 = f[3];
        int32_t f4 = f[4];
        int32_t f5 = f[5];
        int32_t f6 = f[6];
        int32_t f7 = f[7];
        int32_t f8 = f[8];
        int32_t f9 = f[9];

        int32_t g0 = g[0];
        int32_t g1 = g[1];
        int32_t g2 = g[2];
        int32_t g3 = g[3];
        int32_t g4 = g[4];
        int32_t g5 = g[5];
        int32_t g6 = g[6];
        int32_t g7 = g[7];
        int32_t g8 = g[8];
        int32_t g9 = g[9];

        int32_t g1_19 = 19 * g1; /* 1.959375*2^29 */
        int32_t g2_19 = 19 * g2; /* 1.959375*2^30; still ok */
        int32_t g3_19 = 19 * g3;
        int32_t g4_19 = 19 * g4;
        int32_t g5_19 = 19 * g5;
        int32_t g6_19 = 19 * g6;
        int32_t g7_19 = 19 * g7;
        int32_t g8_19 = 19 * g8;
        int32_t g9_19 = 19 * g9;
        int32_t f1_2 = 2 * f1;
        int32_t f3_2 = 2 * f3;
        int32_t f5_2 = 2 * f5;
        int32_t f7_2 = 2 * f7;
        int32_t f9_2 = 2 * f9;

        int64_t f0g0 = f0 * (int64_t)g0;
        int64_t f0g1 = f0 * (int64_t)g1;
        int64_t f0g2 = f0 * (int64_t)g2;
        int64_t f0g3 = f0 * (int64_t)g3;
        int64_t f0g4 = f0 * (int64_t)g4;
        int64_t f0g5 = f0 * (int64_t)g5;
        int64_t f0g6 = f0 * (int64_t)g6;
        int64_t f0g7 = f0 * (int64_t)g7;
        int64_t f0g8 = f0 * (int64_t)g8;
        int64_t f0g9 = f0 * (int64_t)g9;
        int64_t f1g0 = f1 * (int64_t)g0;
        int64_t f1g1_2 = f1_2 * (int64_t)g1;
        int64_t f1g2 = f1 * (int64_t)g2;
        int64_t f1g3_2 = f1_2 * (int64_t)g3;
        int64_t f1g4 = f1 * (int64_t)g4;
        int64_t f1g5_2 = f1_2 * (int64_t)g5;
        int64_t f1g6 = f1 * (int64_t)g6;
        int64_t f1g7_2 = f1_2 * (int64_t)g7;
        int64_t f1g8 = f1 * (int64_t)g8;
        int64_t f1g9_38 = f1_2 * (int64_t)g9_19;
        int64_t f2g0 = f2 * (int64_t)g0;
        int64_t f2g1 = f2 * (int64_t)g1;
        int64_t f2g2 = f2 * (int64_t)g2;
        int64_t f2g3 = f2 * (int64_t)g3;
        int64_t f2g4 = f2 * (int64_t)g4;
        int64_t f2g5 = f2 * (int64_t)g5;
        int64_t f2g6 = f2 * (int64_t)g6;
        int64_t f2g7 = f2 * (int64_t)g7;
        int64_t f2g8_19 = f2 * (int64_t)g8_19;
        int64_t f2g9_19 = f2 * (int64_t)g9_19;
        int64_t f3g0 = f3 * (int64_t)g0;
        int64_t f3g1_2 = f3_2 * (int64_t)g1;
        int64_t f3g2 = f3 * (int64_t)g2;
        int64_t f3g3_2 = f3_2 * (int64_t)g3;
        int64_t f3g4 = f3 * (int64_t)g4;
        int64_t f3g5_2 = f3_2 * (int64_t)g5;
        int64_t f3g6 = f3 * (int64_t)g6;
        int64_t f3g7_38 = f3_2 * (int64_t)g7_19;
        int64_t f3g8_19 = f3 * (int64_t)g8_19;
        int64_t f3g9_38 = f3_2 * (int64_t)g9_19;
        int64_t f4g0 = f4 * (int64_t)g0;
        int64_t f4g1 = f4 * (int64_t)g1;
        int64_t f4g2 = f4 * (int64_t)g2;
        int64_t f4g3 = f4 * (int64_t)g3;
        int64_t f4g4 = f4 * (int64_t)g4;
        int64_t f4g5 = f4 * (int64_t)g5;
        int64_t f4g6_19 = f4 * (int64_t)g6_19;
        int64_t f4g7_19 = f4 * (int64_t)g7_19;
        int64_t f4g8_19 = f4 * (int64_t)g8_19;
        int64_t f4g9_19 = f4 * (int64_t)g9_19;
        int64_t f5g0 = f5 * (int64_t)g0;
        int64_t f5g1_2 = f5_2 * (int64_t)g1;
        int64_t f5g2 = f5 * (int64_t)g2;
        int64_t f5g3_2 = f5_2 * (int64_t)g3;
        int64_t f5g4 = f5 * (int64_t)g4;
        int64_t f5g5_38 = f5_2 * (int64_t)g5_19;
        int64_t f5g6_19 = f5 * (int64_t)g6_19;
        int64_t f5g7_38 = f5_2 * (int64_t)g7_19;
        int64_t f5g8_19 = f5 * (int64_t)g8_19;
        int64_t f5g9_38 = f5_2 * (int64_t)g9_19;
        int64_t f6g0 = f6 * (int64_t)g0;
        int64_t f6g1 = f6 * (int64_t)g1;
        int64_t f6g2 = f6 * (int64_t)g2;
        int64_t f6g3 = f6 * (int64_t)g3;
        int64_t f6g4_19 = f6 * (int64_t)g4_19;
        int64_t f6g5_19 = f6 * (int64_t)g5_19;
        int64_t f6g6_19 = f6 * (int64_t)g6_19;
        int64_t f6g7_19 = f6 * (int64_t)g7_19;
        int64_t f6g8_19 = f6 * (int64_t)g8_19;
        int64_t f6g9_19 = f6 * (int64_t)g9_19;
        int64_t f7g0 = f7 * (int64_t)g0;
        int64_t f7g1_2 = f7_2 * (int64_t)g1;
        int64_t f7g2 = f7 * (int64_t)g2;
        int64_t f7g3_38 = f7_2 * (int64_t)g3_19;
        int64_t f7g4_19 = f7 * (int64_t)g4_19;
        int64_t f7g5_38 = f7_2 * (int64_t)g5_19;
        int64_t f7g6_19 = f7 * (int64_t)g6_19;
        int64_t f7g7_38 = f7_2 * (int64_t)g7_19;
        int64_t f7g8_19 = f7 * (int64_t)g8_19;
        int64_t f7g9_38 = f7_2 * (int64_t)g9_19;
        int64_t f8g0 = f8 * (int64_t)g0;
        int64_t f8g1 = f8 * (int64_t)g1;
        int64_t f8g2_19 = f8 * (int64_t)g2_19;
        int64_t f8g3_19 = f8 * (int64_t)g3_19;
        int64_t f8g4_19 = f8 * (int64_t)g4_19;
        int64_t f8g5_19 = f8 * (int64_t)g5_19;
        int64_t f8g6_19 = f8 * (int64_t)g6_19;
        int64_t f8g7_19 = f8 * (int64_t)g7_19;
        int64_t f8g8_19 = f8 * (int64_t)g8_19;
        int64_t f8g9_19 = f8 * (int64_t)g9_19;
        int64_t f9g0 = f9 * (int64_t)g0;
        int64_t f9g1_38 = f9_2 * (int64_t)g1_19;
        int64_t f9g2_19 = f9 * (int64_t)g2_19;
        int64_t f9g3_38 = f9_2 * (int64_t)g3_19;
        int64_t f9g4_19 = f9 * (int64_t)g4_19;
        int64_t f9g5_38 = f9_2 * (int64_t)g5_19;
        int64_t f9g6_19 = f9 * (int64_t)g6_19;
        int64_t f9g7_38 = f9_2 * (int64_t)g7_19;
        int64_t f9g8_19 = f9 * (int64_t)g8_19;
        int64_t f9g9_38 = f9_2 * (int64_t)g9_19;

        int64_t h0 = f0g0 + f1g9_38 + f2g8_19 + f3g7_38 + f4g6_19 + f5g5_38 +
                     f6g4_19 + f7g3_38 + f8g2_19 + f9g1_38;
        int64_t h1 = f0g1 + f1g0 + f2g9_19 + f3g8_19 + f4g7_19 + f5g6_19 + f6g5_19 +
                     f7g4_19 + f8g3_19 + f9g2_19;
        int64_t h2 = f0g2 + f1g1_2 + f2g0 + f3g9_38 + f4g8_19 + f5g7_38 + f6g6_19 +
                     f7g5_38 + f8g4_19 + f9g3_38;
        int64_t h3 = f0g3 + f1g2 + f2g1 + f3g0 + f4g9_19 + f5g8_19 + f6g7_19 +
                     f7g6_19 + f8g5_19 + f9g4_19;
        int64_t h4 = f0g4 + f1g3_2 + f2g2 + f3g1_2 + f4g0 + f5g9_38 + f6g8_19 +
                     f7g7_38 + f8g6_19 + f9g5_38;
        int64_t h5 = f0g5 + f1g4 + f2g3 + f3g2 + f4g1 + f5g0 + f6g9_19 + f7g8_19 +
                     f8g7_19 + f9g6_19;
        int64_t h6 = f0g6 + f1g5_2 + f2g4 + f3g3_2 + f4g2 + f5g1_2 + f6g0 +
                     f7g9_38 + f8g8_19 + f9g7_38;
        int64_t h7 = f0g7 + f1g6 + f2g5 + f3g4 + f4g3 + f5g2 + f6g1 + f7g0 +
                     f8g9_19 + f9g8_19;
        int64_t h8 = f0g8 + f1g7_2 + f2g6 + f3g5_2 + f4g4 + f5g3_2 + f6g2 + f7g1_2 +
                     f8g0 + f9g9_38;
        int64_t h9 =
            f0g9 + f1g8 + f2g7 + f3g6 + f4g5 + f5g4 + f6g3 + f7g2 + f8g1 + f9g0;

        int64_t carry0;
        int64_t carry1;
        int64_t carry2;
        int64_t carry3;
        int64_t carry4;
        int64_t carry5;
        int64_t carry6;
        int64_t carry7;
        int64_t carry8;
        int64_t carry9;

        /*
         |h0| <= (1.65*1.65*2^52*(1+19+19+19+19)+1.65*1.65*2^50*(38+38+38+38+38))
         i.e. |h0| <= 1.4*2^60; narrower ranges for h2, h4, h6, h8
         |h1| <= (1.65*1.65*2^51*(1+1+19+19+19+19+19+19+19+19))
         i.e. |h1| <= 1.7*2^59; narrower ranges for h3, h5, h7, h9
         */

        carry0 = (h0 + (int64_t)(1L << 25)) >> 26;
        h1 += carry0;
        h0 -= carry0 * ((uint64_t)1L << 26);
        carry4 = (h4 + (int64_t)(1L << 25)) >> 26;
        h5 += carry4;
        h4 -= carry4 * ((uint64_t)1L << 26);
        /* |h0| <= 2^25 */
        /* |h4| <= 2^25 */
        /* |h1| <= 1.71*2^59 */
        /* |h5| <= 1.71*2^59 */

        carry1 = (h1 + (int64_t)(1L << 24)) >> 25;
        h2 += carry1;
        h1 -= carry1 * ((uint64_t)1L << 25);
        carry5 = (h5 + (int64_t)(1L << 24)) >> 25;
        h6 += carry5;
        h5 -= carry5 * ((uint64_t)1L << 25);
        /* |h1| <= 2^24; from now on fits into int32 */
        /* |h5| <= 2^24; from now on fits into int32 */
        /* |h2| <= 1.41*2^60 */
        /* |h6| <= 1.41*2^60 */

        carry2 = (h2 + (int64_t)(1L << 25)) >> 26;
        h3 += carry2;
        h2 -= carry2 * ((uint64_t)1L << 26);
        carry6 = (h6 + (int64_t)(1L << 25)) >> 26;
        h7 += carry6;
        h6 -= carry6 * ((uint64_t)1L << 26);
        /* |h2| <= 2^25; from now on fits into int32 unchanged */
        /* |h6| <= 2^25; from now on fits into int32 unchanged */
        /* |h3| <= 1.71*2^59 */
        /* |h7| <= 1.71*2^59 */

        carry3 = (h3 + (int64_t)(1L << 24)) >> 25;
        h4 += carry3;
        h3 -= carry3 * ((uint64_t)1L << 25);
        carry7 = (h7 + (int64_t)(1L << 24)) >> 25;
        h8 += carry7;
        h7 -= carry7 * ((uint64_t)1L << 25);
        /* |h3| <= 2^24; from now on fits into int32 unchanged */
        /* |h7| <= 2^24; from now on fits into int32 unchanged */
        /* |h4| <= 1.72*2^34 */
        /* |h8| <= 1.41*2^60 */

        carry4 = (h4 + (int64_t)(1L << 25)) >> 26;
        h5 += carry4;
        h4 -= carry4 * ((uint64_t)1L << 26);
        carry8 = (h8 + (int64_t)(1L << 25)) >> 26;
        h9 += carry8;
        h8 -= carry8 * ((uint64_t)1L << 26);
        /* |h4| <= 2^25; from now on fits into int32 unchanged */
        /* |h8| <= 2^25; from now on fits into int32 unchanged */
        /* |h5| <= 1.01*2^24 */
        /* |h9| <= 1.71*2^59 */

        carry9 = (h9 + (int64_t)(1L << 24)) >> 25;
        h0 += carry9 * 19;
        h9 -= carry9 * ((uint64_t)1L << 25);
        /* |h9| <= 2^24; from now on fits into int32 unchanged */
        /* |h0| <= 1.1*2^39 */

        carry0 = (h0 + (int64_t)(1L << 25)) >> 26;
        h1 += carry0;
        h0 -= carry0 * ((uint64_t)1L << 26);
        /* |h0| <= 2^25; from now on fits into int32 unchanged */
        /* |h1| <= 1.01*2^24 */

        h[0] = (int32_t)h0;
        h[1] = (int32_t)h1;
        h[2] = (int32_t)h2;
        h[3] = (int32_t)h3;
        h[4] = (int32_t)h4;
        h[5] = (int32_t)h5;
        h[6] = (int32_t)h6;
        h[7] = (int32_t)h7;
        h[8] = (int32_t)h8;
        h[9] = (int32_t)h9;
    }

    /*
     h = f * f
     Can overlap h with f.
     *
     Preconditions:
     |f| bounded by 1.65*2^26,1.65*2^25,1.65*2^26,1.65*2^25,etc.
     *
     Postconditions:
     |h| bounded by 1.01*2^25,1.01*2^24,1.01*2^25,1.01*2^24,etc.
     */

    static void
    fe25519_sq(fe25519 h, const fe25519 f)
    {
        int32_t f0 = f[0];
        int32_t f1 = f[1];
        int32_t f2 = f[2];
        int32_t f3 = f[3];
        int32_t f4 = f[4];
        int32_t f5 = f[5];
        int32_t f6 = f[6];
        int32_t f7 = f[7];
        int32_t f8 = f[8];
        int32_t f9 = f[9];

        int32_t f0_2 = 2 * f0;
        int32_t f1_2 = 2 * f1;
        int32_t f2_2 = 2 * f2;
        int32_t f3_2 = 2 * f3;
        int32_t f4_2 = 2 * f4;
        int32_t f5_2 = 2 * f5;
        int32_t f6_2 = 2 * f6;
        int32_t f7_2 = 2 * f7;
        int32_t f5_38 = 38 * f5; /* 1.959375*2^30 */
        int32_t f6_19 = 19 * f6; /* 1.959375*2^30 */
        int32_t f7_38 = 38 * f7; /* 1.959375*2^30 */
        int32_t f8_19 = 19 * f8; /* 1.959375*2^30 */
        int32_t f9_38 = 38 * f9; /* 1.959375*2^30 */

        int64_t f0f0 = f0 * (int64_t)f0;
        int64_t f0f1_2 = f0_2 * (int64_t)f1;
        int64_t f0f2_2 = f0_2 * (int64_t)f2;
        int64_t f0f3_2 = f0_2 * (int64_t)f3;
        int64_t f0f4_2 = f0_2 * (int64_t)f4;
        int64_t f0f5_2 = f0_2 * (int64_t)f5;
        int64_t f0f6_2 = f0_2 * (int64_t)f6;
        int64_t f0f7_2 = f0_2 * (int64_t)f7;
        int64_t f0f8_2 = f0_2 * (int64_t)f8;
        int64_t f0f9_2 = f0_2 * (int64_t)f9;
        int64_t f1f1_2 = f1_2 * (int64_t)f1;
        int64_t f1f2_2 = f1_2 * (int64_t)f2;
        int64_t f1f3_4 = f1_2 * (int64_t)f3_2;
        int64_t f1f4_2 = f1_2 * (int64_t)f4;
        int64_t f1f5_4 = f1_2 * (int64_t)f5_2;
        int64_t f1f6_2 = f1_2 * (int64_t)f6;
        int64_t f1f7_4 = f1_2 * (int64_t)f7_2;
        int64_t f1f8_2 = f1_2 * (int64_t)f8;
        int64_t f1f9_76 = f1_2 * (int64_t)f9_38;
        int64_t f2f2 = f2 * (int64_t)f2;
        int64_t f2f3_2 = f2_2 * (int64_t)f3;
        int64_t f2f4_2 = f2_2 * (int64_t)f4;
        int64_t f2f5_2 = f2_2 * (int64_t)f5;
        int64_t f2f6_2 = f2_2 * (int64_t)f6;
        int64_t f2f7_2 = f2_2 * (int64_t)f7;
        int64_t f2f8_38 = f2_2 * (int64_t)f8_19;
        int64_t f2f9_38 = f2 * (int64_t)f9_38;
        int64_t f3f3_2 = f3_2 * (int64_t)f3;
        int64_t f3f4_2 = f3_2 * (int64_t)f4;
        int64_t f3f5_4 = f3_2 * (int64_t)f5_2;
        int64_t f3f6_2 = f3_2 * (int64_t)f6;
        int64_t f3f7_76 = f3_2 * (int64_t)f7_38;
        int64_t f3f8_38 = f3_2 * (int64_t)f8_19;
        int64_t f3f9_76 = f3_2 * (int64_t)f9_38;
        int64_t f4f4 = f4 * (int64_t)f4;
        int64_t f4f5_2 = f4_2 * (int64_t)f5;
        int64_t f4f6_38 = f4_2 * (int64_t)f6_19;
        int64_t f4f7_38 = f4 * (int64_t)f7_38;
        int64_t f4f8_38 = f4_2 * (int64_t)f8_19;
        int64_t f4f9_38 = f4 * (int64_t)f9_38;
        int64_t f5f5_38 = f5 * (int64_t)f5_38;
        int64_t f5f6_38 = f5_2 * (int64_t)f6_19;
        int64_t f5f7_76 = f5_2 * (int64_t)f7_38;
        int64_t f5f8_38 = f5_2 * (int64_t)f8_19;
        int64_t f5f9_76 = f5_2 * (int64_t)f9_38;
        int64_t f6f6_19 = f6 * (int64_t)f6_19;
        int64_t f6f7_38 = f6 * (int64_t)f7_38;
        int64_t f6f8_38 = f6_2 * (int64_t)f8_19;
        int64_t f6f9_38 = f6 * (int64_t)f9_38;
        int64_t f7f7_38 = f7 * (int64_t)f7_38;
        int64_t f7f8_38 = f7_2 * (int64_t)f8_19;
        int64_t f7f9_76 = f7_2 * (int64_t)f9_38;
        int64_t f8f8_19 = f8 * (int64_t)f8_19;
        int64_t f8f9_38 = f8 * (int64_t)f9_38;
        int64_t f9f9_38 = f9 * (int64_t)f9_38;

        int64_t h0 = f0f0 + f1f9_76 + f2f8_38 + f3f7_76 + f4f6_38 + f5f5_38;
        int64_t h1 = f0f1_2 + f2f9_38 + f3f8_38 + f4f7_38 + f5f6_38;
        int64_t h2 = f0f2_2 + f1f1_2 + f3f9_76 + f4f8_38 + f5f7_76 + f6f6_19;
        int64_t h3 = f0f3_2 + f1f2_2 + f4f9_38 + f5f8_38 + f6f7_38;
        int64_t h4 = f0f4_2 + f1f3_4 + f2f2 + f5f9_76 + f6f8_38 + f7f7_38;
        int64_t h5 = f0f5_2 + f1f4_2 + f2f3_2 + f6f9_38 + f7f8_38;
        int64_t h6 = f0f6_2 + f1f5_4 + f2f4_2 + f3f3_2 + f7f9_76 + f8f8_19;
        int64_t h7 = f0f7_2 + f1f6_2 + f2f5_2 + f3f4_2 + f8f9_38;
        int64_t h8 = f0f8_2 + f1f7_4 + f2f6_2 + f3f5_4 + f4f4 + f9f9_38;
        int64_t h9 = f0f9_2 + f1f8_2 + f2f7_2 + f3f6_2 + f4f5_2;

        int64_t carry0;
        int64_t carry1;
        int64_t carry2;
        int64_t carry3;
        int64_t carry4;
        int64_t carry5;
        int64_t carry6;
        int64_t carry7;
        int64_t carry8;
        int64_t carry9;

        carry0 = (h0 + (int64_t)(1L << 25)) >> 26;
        h1 += carry0;
        h0 -= carry0 * ((uint64_t)1L << 26);
        carry4 = (h4 + (int64_t)(1L << 25)) >> 26;
        h5 += carry4;
        h4 -= carry4 * ((uint64_t)1L << 26);

        carry1 = (h1 + (int64_t)(1L << 24)) >> 25;
        h2 += carry1;
        h1 -= carry1 * ((uint64_t)1L << 25);
        carry5 = (h5 + (int64_t)(1L << 24)) >> 25;
        h6 += carry5;
        h5 -= carry5 * ((uint64_t)1L << 25);

        carry2 = (h2 + (int64_t)(1L << 25)) >> 26;
        h3 += carry2;
        h2 -= carry2 * ((uint64_t)1L << 26);
        carry6 = (h6 + (int64_t)(1L << 25)) >> 26;
        h7 += carry6;
        h6 -= carry6 * ((uint64_t)1L << 26);

        carry3 = (h3 + (int64_t)(1L << 24)) >> 25;
        h4 += carry3;
        h3 -= carry3 * ((uint64_t)1L << 25);
        carry7 = (h7 + (int64_t)(1L << 24)) >> 25;
        h8 += carry7;
        h7 -= carry7 * ((uint64_t)1L << 25);

        carry4 = (h4 + (int64_t)(1L << 25)) >> 26;
        h5 += carry4;
        h4 -= carry4 * ((uint64_t)1L << 26);
        carry8 = (h8 + (int64_t)(1L << 25)) >> 26;
        h9 += carry8;
        h8 -= carry8 * ((uint64_t)1L << 26);

        carry9 = (h9 + (int64_t)(1L << 24)) >> 25;
        h0 += carry9 * 19;
        h9 -= carry9 * ((uint64_t)1L << 25);

        carry0 = (h0 + (int64_t)(1L << 25)) >> 26;
        h1 += carry0;
        h0 -= carry0 * ((uint64_t)1L << 26);

        h[0] = (int32_t)h0;
        h[1] = (int32_t)h1;
        h[2] = (int32_t)h2;
        h[3] = (int32_t)h3;
        h[4] = (int32_t)h4;
        h[5] = (int32_t)h5;
        h[6] = (int32_t)h6;
        h[7] = (int32_t)h7;
        h[8] = (int32_t)h8;
        h[9] = (int32_t)h9;
    }

    /*
     h = 2 * f * f
     Can overlap h with f.
     *
     Preconditions:
     |f| bounded by 1.65*2^26,1.65*2^25,1.65*2^26,1.65*2^25,etc.
     *
     Postconditions:
     |h| bounded by 1.01*2^25,1.01*2^24,1.01*2^25,1.01*2^24,etc.
     */

    static void
    fe25519_sq2(fe25519 h, const fe25519 f)
    {
        int32_t f0 = f[0];
        int32_t f1 = f[1];
        int32_t f2 = f[2];
        int32_t f3 = f[3];
        int32_t f4 = f[4];
        int32_t f5 = f[5];
        int32_t f6 = f[6];
        int32_t f7 = f[7];
        int32_t f8 = f[8];
        int32_t f9 = f[9];

        int32_t f0_2 = 2 * f0;
        int32_t f1_2 = 2 * f1;
        int32_t f2_2 = 2 * f2;
        int32_t f3_2 = 2 * f3;
        int32_t f4_2 = 2 * f4;
        int32_t f5_2 = 2 * f5;
        int32_t f6_2 = 2 * f6;
        int32_t f7_2 = 2 * f7;
        int32_t f5_38 = 38 * f5; /* 1.959375*2^30 */
        int32_t f6_19 = 19 * f6; /* 1.959375*2^30 */
        int32_t f7_38 = 38 * f7; /* 1.959375*2^30 */
        int32_t f8_19 = 19 * f8; /* 1.959375*2^30 */
        int32_t f9_38 = 38 * f9; /* 1.959375*2^30 */

        int64_t f0f0 = f0 * (int64_t)f0;
        int64_t f0f1_2 = f0_2 * (int64_t)f1;
        int64_t f0f2_2 = f0_2 * (int64_t)f2;
        int64_t f0f3_2 = f0_2 * (int64_t)f3;
        int64_t f0f4_2 = f0_2 * (int64_t)f4;
        int64_t f0f5_2 = f0_2 * (int64_t)f5;
        int64_t f0f6_2 = f0_2 * (int64_t)f6;
        int64_t f0f7_2 = f0_2 * (int64_t)f7;
        int64_t f0f8_2 = f0_2 * (int64_t)f8;
        int64_t f0f9_2 = f0_2 * (int64_t)f9;
        int64_t f1f1_2 = f1_2 * (int64_t)f1;
        int64_t f1f2_2 = f1_2 * (int64_t)f2;
        int64_t f1f3_4 = f1_2 * (int64_t)f3_2;
        int64_t f1f4_2 = f1_2 * (int64_t)f4;
        int64_t f1f5_4 = f1_2 * (int64_t)f5_2;
        int64_t f1f6_2 = f1_2 * (int64_t)f6;
        int64_t f1f7_4 = f1_2 * (int64_t)f7_2;
        int64_t f1f8_2 = f1_2 * (int64_t)f8;
        int64_t f1f9_76 = f1_2 * (int64_t)f9_38;
        int64_t f2f2 = f2 * (int64_t)f2;
        int64_t f2f3_2 = f2_2 * (int64_t)f3;
        int64_t f2f4_2 = f2_2 * (int64_t)f4;
        int64_t f2f5_2 = f2_2 * (int64_t)f5;
        int64_t f2f6_2 = f2_2 * (int64_t)f6;
        int64_t f2f7_2 = f2_2 * (int64_t)f7;
        int64_t f2f8_38 = f2_2 * (int64_t)f8_19;
        int64_t f2f9_38 = f2 * (int64_t)f9_38;
        int64_t f3f3_2 = f3_2 * (int64_t)f3;
        int64_t f3f4_2 = f3_2 * (int64_t)f4;
        int64_t f3f5_4 = f3_2 * (int64_t)f5_2;
        int64_t f3f6_2 = f3_2 * (int64_t)f6;
        int64_t f3f7_76 = f3_2 * (int64_t)f7_38;
        int64_t f3f8_38 = f3_2 * (int64_t)f8_19;
        int64_t f3f9_76 = f3_2 * (int64_t)f9_38;
        int64_t f4f4 = f4 * (int64_t)f4;
        int64_t f4f5_2 = f4_2 * (int64_t)f5;
        int64_t f4f6_38 = f4_2 * (int64_t)f6_19;
        int64_t f4f7_38 = f4 * (int64_t)f7_38;
        int64_t f4f8_38 = f4_2 * (int64_t)f8_19;
        int64_t f4f9_38 = f4 * (int64_t)f9_38;
        int64_t f5f5_38 = f5 * (int64_t)f5_38;
        int64_t f5f6_38 = f5_2 * (int64_t)f6_19;
        int64_t f5f7_76 = f5_2 * (int64_t)f7_38;
        int64_t f5f8_38 = f5_2 * (int64_t)f8_19;
        int64_t f5f9_76 = f5_2 * (int64_t)f9_38;
        int64_t f6f6_19 = f6 * (int64_t)f6_19;
        int64_t f6f7_38 = f6 * (int64_t)f7_38;
        int64_t f6f8_38 = f6_2 * (int64_t)f8_19;
        int64_t f6f9_38 = f6 * (int64_t)f9_38;
        int64_t f7f7_38 = f7 * (int64_t)f7_38;
        int64_t f7f8_38 = f7_2 * (int64_t)f8_19;
        int64_t f7f9_76 = f7_2 * (int64_t)f9_38;
        int64_t f8f8_19 = f8 * (int64_t)f8_19;
        int64_t f8f9_38 = f8 * (int64_t)f9_38;
        int64_t f9f9_38 = f9 * (int64_t)f9_38;

        int64_t h0 = f0f0 + f1f9_76 + f2f8_38 + f3f7_76 + f4f6_38 + f5f5_38;
        int64_t h1 = f0f1_2 + f2f9_38 + f3f8_38 + f4f7_38 + f5f6_38;
        int64_t h2 = f0f2_2 + f1f1_2 + f3f9_76 + f4f8_38 + f5f7_76 + f6f6_19;
        int64_t h3 = f0f3_2 + f1f2_2 + f4f9_38 + f5f8_38 + f6f7_38;
        int64_t h4 = f0f4_2 + f1f3_4 + f2f2 + f5f9_76 + f6f8_38 + f7f7_38;
        int64_t h5 = f0f5_2 + f1f4_2 + f2f3_2 + f6f9_38 + f7f8_38;
        int64_t h6 = f0f6_2 + f1f5_4 + f2f4_2 + f3f3_2 + f7f9_76 + f8f8_19;
        int64_t h7 = f0f7_2 + f1f6_2 + f2f5_2 + f3f4_2 + f8f9_38;
        int64_t h8 = f0f8_2 + f1f7_4 + f2f6_2 + f3f5_4 + f4f4 + f9f9_38;
        int64_t h9 = f0f9_2 + f1f8_2 + f2f7_2 + f3f6_2 + f4f5_2;

        int64_t carry0;
        int64_t carry1;
        int64_t carry2;
        int64_t carry3;
        int64_t carry4;
        int64_t carry5;
        int64_t carry6;
        int64_t carry7;
        int64_t carry8;
        int64_t carry9;

        h0 += h0;
        h1 += h1;
        h2 += h2;
        h3 += h3;
        h4 += h4;
        h5 += h5;
        h6 += h6;
        h7 += h7;
        h8 += h8;
        h9 += h9;

        carry0 = (h0 + (int64_t)(1L << 25)) >> 26;
        h1 += carry0;
        h0 -= carry0 * ((uint64_t)1L << 26);
        carry4 = (h4 + (int64_t)(1L << 25)) >> 26;
        h5 += carry4;
        h4 -= carry4 * ((uint64_t)1L << 26);

        carry1 = (h1 + (int64_t)(1L << 24)) >> 25;
        h2 += carry1;
        h1 -= carry1 * ((uint64_t)1L << 25);
        carry5 = (h5 + (int64_t)(1L << 24)) >> 25;
        h6 += carry5;
        h5 -= carry5 * ((uint64_t)1L << 25);

        carry2 = (h2 + (int64_t)(1L << 25)) >> 26;
        h3 += carry2;
        h2 -= carry2 * ((uint64_t)1L << 26);
        carry6 = (h6 + (int64_t)(1L << 25)) >> 26;
        h7 += carry6;
        h6 -= carry6 * ((uint64_t)1L << 26);

        carry3 = (h3 + (int64_t)(1L << 24)) >> 25;
        h4 += carry3;
        h3 -= carry3 * ((uint64_t)1L << 25);
        carry7 = (h7 + (int64_t)(1L << 24)) >> 25;
        h8 += carry7;
        h7 -= carry7 * ((uint64_t)1L << 25);

        carry4 = (h4 + (int64_t)(1L << 25)) >> 26;
        h5 += carry4;
        h4 -= carry4 * ((uint64_t)1L << 26);
        carry8 = (h8 + (int64_t)(1L << 25)) >> 26;
        h9 += carry8;
        h8 -= carry8 * ((uint64_t)1L << 26);

        carry9 = (h9 + (int64_t)(1L << 24)) >> 25;
        h0 += carry9 * 19;
        h9 -= carry9 * ((uint64_t)1L << 25);

        carry0 = (h0 + (int64_t)(1L << 25)) >> 26;
        h1 += carry0;
        h0 -= carry0 * ((uint64_t)1L << 26);

        h[0] = (int32_t)h0;
        h[1] = (int32_t)h1;
        h[2] = (int32_t)h2;
        h[3] = (int32_t)h3;
        h[4] = (int32_t)h4;
        h[5] = (int32_t)h5;
        h[6] = (int32_t)h6;
        h[7] = (int32_t)h7;
        h[8] = (int32_t)h8;
        h[9] = (int32_t)h9;
    }

    static inline void
    fe25519_mul32(fe25519 h, const fe25519 f, uint32_t n)
    {
        int64_t sn = (int64_t)n;
        int32_t f0 = f[0];
        int32_t f1 = f[1];
        int32_t f2 = f[2];
        int32_t f3 = f[3];
        int32_t f4 = f[4];
        int32_t f5 = f[5];
        int32_t f6 = f[6];
        int32_t f7 = f[7];
        int32_t f8 = f[8];
        int32_t f9 = f[9];
        int64_t h0 = f0 * sn;
        int64_t h1 = f1 * sn;
        int64_t h2 = f2 * sn;
        int64_t h3 = f3 * sn;
        int64_t h4 = f4 * sn;
        int64_t h5 = f5 * sn;
        int64_t h6 = f6 * sn;
        int64_t h7 = f7 * sn;
        int64_t h8 = f8 * sn;
        int64_t h9 = f9 * sn;
        int64_t carry0, carry1, carry2, carry3, carry4, carry5, carry6, carry7,
            carry8, carry9;

        carry9 = (h9 + ((int64_t)1 << 24)) >> 25;
        h0 += carry9 * 19;
        h9 -= carry9 * ((int64_t)1 << 25);
        carry1 = (h1 + ((int64_t)1 << 24)) >> 25;
        h2 += carry1;
        h1 -= carry1 * ((int64_t)1 << 25);
        carry3 = (h3 + ((int64_t)1 << 24)) >> 25;
        h4 += carry3;
        h3 -= carry3 * ((int64_t)1 << 25);
        carry5 = (h5 + ((int64_t)1 << 24)) >> 25;
        h6 += carry5;
        h5 -= carry5 * ((int64_t)1 << 25);
        carry7 = (h7 + ((int64_t)1 << 24)) >> 25;
        h8 += carry7;
        h7 -= carry7 * ((int64_t)1 << 25);

        carry0 = (h0 + ((int64_t)1 << 25)) >> 26;
        h1 += carry0;
        h0 -= carry0 * ((int64_t)1 << 26);
        carry2 = (h2 + ((int64_t)1 << 25)) >> 26;
        h3 += carry2;
        h2 -= carry2 * ((int64_t)1 << 26);
        carry4 = (h4 + ((int64_t)1 << 25)) >> 26;
        h5 += carry4;
        h4 -= carry4 * ((int64_t)1 << 26);
        carry6 = (h6 + ((int64_t)1 << 25)) >> 26;
        h7 += carry6;
        h6 -= carry6 * ((int64_t)1 << 26);
        carry8 = (h8 + ((int64_t)1 << 25)) >> 26;
        h9 += carry8;
        h8 -= carry8 * ((int64_t)1 << 26);

        h[0] = (int32_t)h0;
        h[1] = (int32_t)h1;
        h[2] = (int32_t)h2;
        h[3] = (int32_t)h3;
        h[4] = (int32_t)h4;
        h[5] = (int32_t)h5;
        h[6] = (int32_t)h6;
        h[7] = (int32_t)h7;
        h[8] = (int32_t)h8;
        h[9] = (int32_t)h9;
    }

    /*
     * Reject small order points early to mitigate the implications of
     * unexpected optimizations that would affect the ref10 code.
     * See https://eprint.iacr.org/2017/806.pdf for reference.
     */
    static int
    has_small_order(const unsigned char s[32])
    {
        CRYPTO_ALIGN(16)
        static const unsigned char blocklist[][32] = {
            /* 0 (order 4) */
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
            /* 1 (order 1) */
            {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
            /* 325606250916557431795983626356110631294008115727848805560023387167927233504
               (order 8) */
            {0xe0, 0xeb, 0x7a, 0x7c, 0x3b, 0x41, 0xb8, 0xae, 0x16, 0x56, 0xe3,
             0xfa, 0xf1, 0x9f, 0xc4, 0x6a, 0xda, 0x09, 0x8d, 0xeb, 0x9c, 0x32,
             0xb1, 0xfd, 0x86, 0x62, 0x05, 0x16, 0x5f, 0x49, 0xb8, 0x00},
            /* 39382357235489614581723060781553021112529911719440698176882885853963445705823
               (order 8) */
            {0x5f, 0x9c, 0x95, 0xbc, 0xa3, 0x50, 0x8c, 0x24, 0xb1, 0xd0, 0xb1,
             0x55, 0x9c, 0x83, 0xef, 0x5b, 0x04, 0x44, 0x5c, 0xc4, 0x58, 0x1c,
             0x8e, 0x86, 0xd8, 0x22, 0x4e, 0xdd, 0xd0, 0x9f, 0x11, 0x57},
            /* p-1 (order 2) */
            {0xec, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
             0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
             0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f},
            /* p (=0, order 4) */
            {0xed, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
             0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
             0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f},
            /* p+1 (=1, order 1) */
            {0xee, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
             0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
             0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f}};
        unsigned char c[7] = {0};
        unsigned int k;
        size_t i, j;

        COMPILER_ASSERT(7 == sizeof blocklist / sizeof blocklist[0]);
        for (j = 0; j < 31; j++)
        {
            for (i = 0; i < sizeof blocklist / sizeof blocklist[0]; i++)
            {
                c[i] |= s[j] ^ blocklist[i][j];
            }
        }
        for (i = 0; i < sizeof blocklist / sizeof blocklist[0]; i++)
        {
            c[i] |= (s[j] & 0x7f) ^ blocklist[i][j];
        }
        k = 0;
        for (i = 0; i < sizeof blocklist / sizeof blocklist[0]; i++)
        {
            k |= (c[i] - 1);
        }
        return (int)((k >> 8) & 1);
    }

    // static
    int
    crypto_scalarmult_curve25519_ref10(unsigned char *q,
                                       const unsigned char *n,
                                       const unsigned char *p)
    {
        unsigned char *t = q;
        unsigned int i;
        fe25519 x1, x2, x3, z2, z3;
        fe25519 a, b, aa, bb, e, da, cb;
        int pos;
        unsigned int swap;
        unsigned int bit;

        if (has_small_order(p))
        {
            return -1;
        }
        for (i = 0; i < 32; i++)
        {
            t[i] = n[i];
        }
        t[0] &= 248;
        t[31] &= 127;
        t[31] |= 64;
        fe25519_frombytes(x1, p);
        fe25519_1(x2);
        fe25519_0(z2);
        fe25519_copy(x3, x1);
        fe25519_1(z3);

        swap = 0;
        for (pos = 254; pos >= 0; --pos)
        {
            bit = t[pos / 8] >> (pos & 7);
            bit &= 1;
            swap ^= bit;
            fe25519_cswap(x2, x3, swap);
            fe25519_cswap(z2, z3, swap);
            swap = bit;
            fe25519_add(a, x2, z2);
            fe25519_sub(b, x2, z2);
            fe25519_sq(aa, a);
            fe25519_sq(bb, b);
            fe25519_mul(x2, aa, bb);
            fe25519_sub(e, aa, bb);
            fe25519_sub(da, x3, z3);
            fe25519_mul(da, da, a);
            fe25519_add(cb, x3, z3);
            fe25519_mul(cb, cb, b);
            fe25519_add(x3, da, cb);
            fe25519_sq(x3, x3);
            fe25519_sub(z3, da, cb);
            fe25519_sq(z3, z3);
            fe25519_mul(z3, z3, x1);
            fe25519_mul32(z2, e, 121666);
            fe25519_add(z2, z2, bb);
            fe25519_mul(z2, z2, e);
        }
        fe25519_cswap(x2, x3, swap);
        fe25519_cswap(z2, z3, swap);

        fe25519_invert(z2, z2);
        fe25519_mul(x2, x2, z2);
        fe25519_tobytes(q, x2);

        return 0;
    }

    /*
     * Inversion - returns 0 if z=0
     */
    void
    fe25519_invert(fe25519 out, const fe25519 z)
    {
        fe25519 t0, t1, t2, t3;
        int i;

        fe25519_sq(t0, z);
        fe25519_sq(t1, t0);
        fe25519_sq(t1, t1);
        fe25519_mul(t1, z, t1);
        fe25519_mul(t0, t0, t1);
        fe25519_sq(t2, t0);
        fe25519_mul(t1, t1, t2);
        fe25519_sq(t2, t1);
        for (i = 1; i < 5; ++i)
        {
            fe25519_sq(t2, t2);
        }
        fe25519_mul(t1, t2, t1);
        fe25519_sq(t2, t1);
        for (i = 1; i < 10; ++i)
        {
            fe25519_sq(t2, t2);
        }
        fe25519_mul(t2, t2, t1);
        fe25519_sq(t3, t2);
        for (i = 1; i < 20; ++i)
        {
            fe25519_sq(t3, t3);
        }
        fe25519_mul(t2, t3, t2);
        for (i = 1; i < 11; ++i)
        {
            fe25519_sq(t2, t2);
        }
        fe25519_mul(t1, t2, t1);
        fe25519_sq(t2, t1);
        for (i = 1; i < 50; ++i)
        {
            fe25519_sq(t2, t2);
        }
        fe25519_mul(t2, t2, t1);
        fe25519_sq(t3, t2);
        for (i = 1; i < 100; ++i)
        {
            fe25519_sq(t3, t3);
        }
        fe25519_mul(t2, t3, t2);
        for (i = 1; i < 51; ++i)
        {
            fe25519_sq(t2, t2);
        }
        fe25519_mul(t1, t2, t1);
        for (i = 1; i < 6; ++i)
        {
            fe25519_sq(t1, t1);
        }
        fe25519_mul(out, t1, t0);
    }

    static void
    edwards_to_montgomery(fe25519 montgomeryX, const fe25519 edwardsY, const fe25519 edwardsZ)
    {
        fe25519 tempX;
        fe25519 tempZ;

        fe25519_add(tempX, edwardsZ, edwardsY);
        fe25519_sub(tempZ, edwardsZ, edwardsY);
        fe25519_invert(tempZ, tempZ);
        fe25519_mul(montgomeryX, tempX, tempZ);
    }

    void
    ge25519_scalarmult_base(ge25519_p3 *h, const unsigned char *a);

    // static
    int
    crypto_scalarmult_curve25519_ref10_base(unsigned char *q,
                                            const unsigned char *n)
    {
        unsigned char *t = q;
        ge25519_p3 A;
        fe25519 pk;
        unsigned int i;

        for (i = 0; i < 32; i++)
        {
            t[i] = n[i];
        }
        t[0] &= 248;
        t[31] &= 127;
        t[31] |= 64;
        ge25519_scalarmult_base(&A, t);
        edwards_to_montgomery(pk, A.Y, A.Z);
        fe25519_tobytes(q, pk);

        return 0;
    }

    /*
     ge means group element.

     Here the group is the set of pairs (x,y) of field elements
     satisfying -x^2 + y^2 = 1 + d x^2y^2
     where d = -121665/121666.

     Representations:
     ge25519_p2 (projective): (X:Y:Z) satisfying x=X/Z, y=Y/Z
     ge25519_p3 (extended): (X:Y:Z:T) satisfying x=X/Z, y=Y/Z, XY=ZT
     ge25519_p1p1 (completed): ((X:Z),(Y:T)) satisfying x=X/Z, y=Y/T
     ge25519_precomp (Duif): (y+x,y-x,2dxy)
     */

    typedef struct
    {
        fe25519 X;
        fe25519 Y;
        fe25519 Z;
    } ge25519_p2;

    // typedef struct {
    //     fe25519 X;
    //     fe25519 Y;
    //     fe25519 Z;
    //     fe25519 T;
    // } ge25519_p3;

    typedef struct
    {
        fe25519 X;
        fe25519 Y;
        fe25519 Z;
        fe25519 T;
    } ge25519_p1p1;

    typedef struct
    {
        fe25519 yplusx;
        fe25519 yminusx;
        fe25519 xy2d;
    } ge25519_precomp;

    typedef struct
    {
        fe25519 YplusX;
        fe25519 YminusX;
        fe25519 Z;
        fe25519 T2d;
    } ge25519_cached;

    static void
    ge25519_p3_0(ge25519_p3 *h)
    {
        fe25519_0(h->X);
        fe25519_1(h->Y);
        fe25519_1(h->Z);
        fe25519_0(h->T);
    }

    static void
    ge25519_precomp_0(ge25519_precomp *h)
    {
        fe25519_1(h->yplusx);
        fe25519_1(h->yminusx);
        fe25519_0(h->xy2d);
    }

    static unsigned char
    negative(signed char b)
    {
        /* 18446744073709551361..18446744073709551615: yes; 0..255: no */
        uint64_t x = b;

        x >>= 63; /* 1: yes; 0: no */

        return x;
    }

    static void
    ge25519_cmov(ge25519_precomp *t, const ge25519_precomp *u, unsigned char b)
    {
        fe25519_cmov(t->yplusx, u->yplusx, b);
        fe25519_cmov(t->yminusx, u->yminusx, b);
        fe25519_cmov(t->xy2d, u->xy2d, b);
    }

    static unsigned char
    equal(signed char b, signed char c)
    {
        unsigned char ub = b;
        unsigned char uc = c;
        unsigned char x = ub ^ uc; /* 0: yes; 1..255: no */
        uint32_t y = (uint32_t)x;  /* 0: yes; 1..255: no */

        y -= 1;   /* 4294967295: yes; 0..254: no */
        y >>= 31; /* 1: yes; 0: no */

        return y;
    }

    static void
    ge25519_cmov8(ge25519_precomp *t, const ge25519_precomp precomp[8], const signed char b)
    {
        ge25519_precomp minust;
        const unsigned char bnegative = negative(b);
        const unsigned char babs = b - (((-bnegative) & b) * ((signed char)1 << 1));

        ge25519_precomp_0(t);
        ge25519_cmov(t, &precomp[0], equal(babs, 1));
        ge25519_cmov(t, &precomp[1], equal(babs, 2));
        ge25519_cmov(t, &precomp[2], equal(babs, 3));
        ge25519_cmov(t, &precomp[3], equal(babs, 4));
        ge25519_cmov(t, &precomp[4], equal(babs, 5));
        ge25519_cmov(t, &precomp[5], equal(babs, 6));
        ge25519_cmov(t, &precomp[6], equal(babs, 7));
        ge25519_cmov(t, &precomp[7], equal(babs, 8));
        fe25519_copy(minust.yplusx, t->yminusx);
        fe25519_copy(minust.yminusx, t->yplusx);
        fe25519_neg(minust.xy2d, t->xy2d);
        ge25519_cmov(t, &minust, bnegative);
    }

    static void
    ge25519_cmov8_base(ge25519_precomp *t, const int pos, const signed char b)
    {
        static const ge25519_precomp PROGMEM base[32][8] = {
        /* base[i][j] = { (j+1)*256^i*B  */
// #ifdef HAVE_TI_MODE
// # include "fe_51/base.h"
// #else
#include "crypto/base.h"
            // #endif
        };
        ge25519_cmov8(t, base[pos], b);
    }

    /*
     r = p + q
     */

    static void
    ge25519_add_precomp(ge25519_p1p1 *r, const ge25519_p3 *p, const ge25519_precomp *q)
    {
        fe25519 t0;

        fe25519_add(r->X, p->Y, p->X);
        fe25519_sub(r->Y, p->Y, p->X);
        fe25519_mul(r->Z, r->X, q->yplusx);
        fe25519_mul(r->Y, r->Y, q->yminusx);
        fe25519_mul(r->T, q->xy2d, p->T);
        fe25519_add(t0, p->Z, p->Z);
        fe25519_sub(r->X, r->Z, r->Y);
        fe25519_add(r->Y, r->Z, r->Y);
        fe25519_add(r->Z, t0, r->T);
        fe25519_sub(r->T, t0, r->T);
    }

    /*
     r = p
     */

    void
    ge25519_p1p1_to_p3(ge25519_p3 *r, const ge25519_p1p1 *p)
    {
        fe25519_mul(r->X, p->X, p->T);
        fe25519_mul(r->Y, p->Y, p->Z);
        fe25519_mul(r->Z, p->Z, p->T);
        fe25519_mul(r->T, p->X, p->Y);
    }

    /*
     r = p
     */

    static void
    ge25519_p3_to_p2(ge25519_p2 *r, const ge25519_p3 *p)
    {
        fe25519_copy(r->X, p->X);
        fe25519_copy(r->Y, p->Y);
        fe25519_copy(r->Z, p->Z);
    }

    /*
     r = 2 * p
     */

    static void
    ge25519_p2_dbl(ge25519_p1p1 *r, const ge25519_p2 *p)
    {
        fe25519 t0;

        fe25519_sq(r->X, p->X);
        fe25519_sq(r->Z, p->Y);
        fe25519_sq2(r->T, p->Z);
        fe25519_add(r->Y, p->X, p->Y);
        fe25519_sq(t0, r->Y);
        fe25519_add(r->Y, r->Z, r->X);
        fe25519_sub(r->Z, r->Z, r->X);
        fe25519_sub(r->X, t0, r->Y);
        fe25519_sub(r->T, r->T, r->Z);
    }

    /*
     r = 2 * p
     */

    static void
    ge25519_p3_dbl(ge25519_p1p1 *r, const ge25519_p3 *p)
    {
        ge25519_p2 q;
        ge25519_p3_to_p2(&q, p);
        ge25519_p2_dbl(r, &q);
    }

    /*
     r = p
     */

    void
    ge25519_p1p1_to_p2(ge25519_p2 *r, const ge25519_p1p1 *p)
    {
        fe25519_mul(r->X, p->X, p->T);
        fe25519_mul(r->Y, p->Y, p->Z);
        fe25519_mul(r->Z, p->Z, p->T);
    }

    /*
 h = a * B (with precomputation)
 where a = a[0]+256*a[1]+...+256^31 a[31]
 B is the Ed25519 base point (x,4/5) with x positive
 (as bytes: 0x5866666666666666666666666666666666666666666666666666666666666666)

 Preconditions:
 a[31] <= 127
 */

    // static
    // int
    // crypto_scalarmult_curve25519_ref10_base(unsigned char *q,
    //                                         const unsigned char *n)
    // {
    //     unsigned char *t = q;
    //     ge25519_p3     A;
    //     fe25519        pk;
    //     unsigned int   i;

    //     for (i = 0; i < 32; i++) {
    //         t[i] = n[i];
    //     }
    //     t[0] &= 248;
    //     t[31] &= 127;
    //     t[31] |= 64;
    //     ge25519_scalarmult_base(&A, t);
    //     edwards_to_montgomery(pk, A.Y, A.Z);
    //     fe25519_tobytes(q, pk);

    //     return 0;
    // }
    void
    ge25519_scalarmult_base(ge25519_p3 *h, const unsigned char *a)
    {
        signed char e[64];
        signed char carry;
        ge25519_p1p1 r;
        ge25519_p2 s;
        ge25519_precomp t;
        int i;

        for (i = 0; i < 32; ++i)
        {
            e[2 * i + 0] = (a[i] >> 0) & 15;
            e[2 * i + 1] = (a[i] >> 4) & 15;
        }
        /* each e[i] is between 0 and 15 */
        /* e[63] is between 0 and 7 */

        carry = 0;
        for (i = 0; i < 63; ++i)
        {
            e[i] += carry;
            carry = e[i] + 8;
            carry >>= 4;
            e[i] -= carry * ((signed char)1 << 4);
        }
        e[63] += carry;
        /* each e[i] is between -8 and 8 */

        ge25519_p3_0(h);

        for (i = 1; i < 64; i += 2)
        {
            ge25519_cmov8_base(&t, i / 2, e[i]);
            ge25519_add_precomp(&r, h, &t);
            ge25519_p1p1_to_p3(h, &r);
        }

        ge25519_p3_dbl(&r, h);
        ge25519_p1p1_to_p2(&s, &r);
        ge25519_p2_dbl(&r, &s);
        ge25519_p1p1_to_p2(&s, &r);
        ge25519_p2_dbl(&r, &s);
        ge25519_p1p1_to_p2(&s, &r);
        ge25519_p2_dbl(&r, &s);
        ge25519_p1p1_to_p3(h, &r);

        for (i = 0; i < 64; i += 2)
        {
            ge25519_cmov8_base(&t, i / 2, e[i]);
            ge25519_add_precomp(&r, h, &t);
            ge25519_p1p1_to_p3(h, &r);
        }
    }

    // struct crypto_scalarmult_curve25519_implementation
    //     crypto_scalarmult_curve25519_ref10_implementation = {
    //         SODIUM_C99(.mult =) crypto_scalarmult_curve25519_ref10,
    //         SODIUM_C99(.mult_base =) crypto_scalarmult_curve25519_ref10_base};

    // typedef struct crypto_scalarmult_curve25519_implementation
    // {
    //     int mult(unsigned char *q, const unsigned char *n,
    //                 const unsigned char *p){
    //                 return  crypto_scalarmult_curve25519_ref10 (q,n,p);
    //                 }
    //     int mult_base(unsigned char *q, const unsigned char *n){
    //             return crypto_scalarmult_curve25519_ref10_base(q,n)
    //     }

    // } crypto_scalarmult_curve25519_implementation;

}