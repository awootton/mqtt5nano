
/** This is not copyright by me. I found it. Portions are from libsodium.
 */

#pragma once

#include <string.h>
#include <stdint.h>

namespace knotcrypto
{
    class Sha256Class
    {
    public:
        /** Initialized the SHA256 process
         *  Set the counter and buffers
         */
        void init(void);

        /**
         * Adds the string to the buffer
         * Calles function in order to add the data into the buffer.
         * @param *buffer The string to be added
         * @param *size The size of the string
         * @note Print class does not exist in linux, so we define it here using
         * @code #if defined(SHA1_LINUX) @endcode
         *
         */
        size_t write(uint8_t data);

        /** Pads the last block and finalizes the hash.
         *
         * @return returns the hash
         */
        uint8_t *result(void);

        // isn't this all below private?
        /** Implement SHA-1 padding (fips180-2 ยง5.1.1).
         *  Pad with 0x80 followed by 0x00 until the end of the block
         *
         */
        void pad();

        /** adds the data to the buffer
         *
         * @param data
         *
         */
        void addUncounted(uint8_t data);

        /** Hash a single block of data
         *
         */
        void hashBlock();

#define HASH_LENGTH 32
#define BLOCK_LENGTH 64

        union _buffer
        {
            uint8_t b[BLOCK_LENGTH];
            uint32_t w[BLOCK_LENGTH / 4];
        };
        union _state
        {
            uint8_t b[HASH_LENGTH];
            uint32_t w[HASH_LENGTH / 4];
        };

        /**
         * ror32 - rotate a 32-bit value left
         * @param number value to rotate
         * @param bits bits to roll
         */
        uint32_t ror32(uint32_t number, uint8_t bits);
        _buffer buffer;                  /**< hold the buffer for the hashing process */
        uint8_t bufferOffset;            /**< indicates the position on the buffer */
        _state state;                    /**< identical structure with buffer */
        uint32_t byteCount;              /**< Byte counter in order to initialize the hash process for a block */
        uint8_t keyBuffer[BLOCK_LENGTH]; /**< Hold the key for the HMAC process*/
        uint8_t innerHash[HASH_LENGTH];  /**< holds the inner hash for the HMAC process */
    };

}