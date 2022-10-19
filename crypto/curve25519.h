

/** This is not copyright by me. Portions are from libsodium.
 */

#pragma once

#include <string.h>
#include <stdint.h>

namespace knotcrypto
{

    extern int crypto_scalarmult_curve25519_ref10(unsigned char *q, const unsigned char *n,
                                                 const unsigned char *p);

    extern int crypto_scalarmult_curve25519_ref10_base(unsigned char *q, const unsigned char *n);

    // typedef struct crypto_scalarmult_curve25519_implementation
    // {
    //     // int mult(unsigned char *q, const unsigned char *n,
    //     //             const unsigned char *p);
    //     // int mult_base(unsigned char *q, const unsigned char *n);

    //     int mult(unsigned char *q, const unsigned char *n,
    //              const unsigned char *p)
    //     {
    //         return crypto_scalarmult_curve25519_ref10(q, n, p);
    //     }

    //     int mult_base(unsigned char *q, const unsigned char *n)
    //     {
    //         return crypto_scalarmult_curve25519_ref10_base(q, n);
    //     }

    // } crypto_scalarmult_curve25519_implementation;

    // extern struct crypto_scalarmult_curve25519_implementation
    // crypto_scalarmult_curve25519_ref10_implementation;

}