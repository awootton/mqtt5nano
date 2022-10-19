/*
	poly1305 implementation using 16 bit * 16 bit = 32 bit multiplication and 32 bit addition
    see: https://github.com/floodyberry/poly1305-donna
*/

#pragma once 

#include <stdint.h>

#include <stddef.h>

namespace knotcrypto {

    // typedef int size_t;

#ifndef CRYPTO_ALIGN
#if defined(__INTEL_COMPILER) || defined(_MSC_VER)
#define CRYPTO_ALIGN(x) __declspec(align(x))
#else
#define CRYPTO_ALIGN(x) __attribute__((aligned(x)))
#endif
#endif


#define poly1305_block_size 16

    /* 17 + sizeof(size_t) + 18*sizeof(unsigned short) */
typedef struct poly1305_state_internal_t {
	unsigned char buffer[poly1305_block_size];
	size_t leftover;
	unsigned short r[10];
	unsigned short h[10];
	unsigned short pad[8];
	unsigned char final;
} poly1305_state_internal_t;


typedef struct poly1305_context {
	size_t aligner;
	unsigned char opaque[136];
} poly1305_context;

void poly1305_init(poly1305_context *ctx, const unsigned char key[32]);
void poly1305_update(poly1305_context *ctx, const unsigned char *m, size_t bytes);
void poly1305_finish(poly1305_context *ctx, unsigned char mac[16]);
void poly1305_auth(unsigned char mac[16], const unsigned char *m, size_t bytes, const unsigned char key[32]);

int poly1305_verify(const unsigned char mac1[16], const unsigned char mac2[16]);
int poly1305_power_on_self_test(void);

static int
crypto_onetimeauth_poly1305_donna(unsigned char *out, const unsigned char *m,
                                  unsigned long long   inlen,
                                  const unsigned char *key)
{
   // poly1305_state_internal_t state;
     CRYPTO_ALIGN(64) poly1305_context state;

    poly1305_init(&state, key);
    poly1305_update(&state, m, inlen);
    poly1305_finish(&state, out);

    return 0;
}

int
crypto_verify_16(const unsigned char *x, const unsigned char *y)
{
   // return crypto_verify_n(x, y, crypto_verify_16_BYTES);
    return poly1305_verify(x, y);
}

static int
crypto_onetimeauth_poly1305_donna_verify(const unsigned char *h,
                                         const unsigned char *in,
                                         unsigned long long   inlen,
                                         const unsigned char *k)
{
    unsigned char correct[16];

    crypto_onetimeauth_poly1305_donna(correct, in, inlen, k);

    return crypto_verify_16(h, correct);
}

int
crypto_onetimeauth_poly1305_verify(const unsigned char *h,
                                   const unsigned char *in,
                                   unsigned long long   inlen,
                                   const unsigned char *k)
{
    //return implementation->onetimeauth_verify(h, in, inlen, k);
    return crypto_onetimeauth_poly1305_donna_verify(h, in, inlen, k);
}


// static int
// crypto_onetimeauth_poly1305_donna(unsigned char *out, const unsigned char *m,
//                                   unsigned long long   inlen,
//                                   const unsigned char *key)
// {
//     CRYPTO_ALIGN(64) 
//     //poly1305_state_internal_t 
//     poly1305_context state;

//     poly1305_init(&state, key);
//     poly1305_update(&state, m, inlen);
//     poly1305_finish(&state, out);

//     return 0;
// }


int
crypto_onetimeauth_poly1305(unsigned char *out, const unsigned char *in,
                            unsigned long long inlen, const unsigned char *k)
{
    //return implementation->onetimeauth(out, in, inlen, k);
    return crypto_onetimeauth_poly1305_donna(out, in, inlen, k);
}


}
