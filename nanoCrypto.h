
/** Libraries are from https://github.com/jedisct1/libsodium and https://github.com/floodyberry/poly1305-donna.
 * This part is copyright Alan Tracey Wootton. See license file.
 */

#pragma once

#include "nanoCommon.h"

// #if defined(ARDUINO)
//     #if defined(ESP32)
//     #define PROGMEM
//     #endif
// #else
// #define PROGMEM
// #endif


#include <stdint.h>
#include <string.h>

#include "slices.h"

#include "crypto/Sha256.h"

#include "crypto/curve25519.h"

using namespace mqtt5nano;

namespace nanocrypto {

    void Sha256Bytes_i(char dest[32], const char *source, int length);

    void Sha256FromString(char dest[32], const char *source);

    void Sha256Bytes(char dest[32], slice source);

    void Curve25519Mpy(char dest[32], const char n[32], const char p[32]);

    void Curve25519BaseMpy(char dest[32], const char n[32]);

    // use slice instead of cstr.
    void getBoxKeyPairFromPassphrase(const char *password, char publicKey[32], char privateKey[32]);

    bool box(ByteCollector *destination, slice message, char (&nonce)[24], char (&pubkey)[32], char (&privkey)[32]);

    bool unbox(ByteCollector *destination, slice encrypted, char (&nonce)[24], char (&pubkey)[32], char (&privkey)[32]);

}