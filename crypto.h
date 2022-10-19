
/** Libraries are from https://github.com/jedisct1/libsodium and https://github.com/floodyberry/poly1305-donna.
 * This part is copyright Alan Tracey Wootton. See license file.
 */

#pragma once

#include <string.h>
#include <stdint.h>

#include "crypto/Sha256.h"

#include "crypto/curve25519.h"

#include "slices.h"

using namespace mqtt5nano;

namespace knotcrypto
{

    void Sha256Bytes(char dest[32], const char *source, int length);

    void Sha256FromString(char dest[32], const char *source);

    void Sha256Bytes(char dest[32], slice source);

    void Curve25519Mpy(char dest[32], const char n[32], const char p[32]);

    void Curve25519BaseMpy(char dest[32], const char n[32]);

    // use slice instead of cstr.
    void getBoxKeyPairFromPassphrase(const char *password, char publicKey[32], char privateKey[32]);

    bool box(sink *destination, slice message, char nonce[24], char pubkey[32], char privkey[32]);

    bool unbox(sink *destination, slice encrypted, char nonce[24], char pubkey[32], char privkey[32]);

}