
namespace base64 {

    // this name is allowed:
    // ABCD*FGHI&JLMN^PQRST%XYZabcdefghijk#yz0@123456789-aaaa.xyz
    // But this one is not
    // A_CD*FGHI&JLMN^PQRST%XYZabcdefghijk#yz0@123456789-aaaa.xyz
    //
    // TODO: use F to put this in flash space
    // or, since it's const, is it not already
    // in code space?  ?? PROGMEM


    const char encodeB64URLTable[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    // const char  encodeStd         = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    unsigned char decodeTable[256];// 128 ?

    void initdecodetable() {
        if (decodeTable['E'] == (unsigned char)4) {
            return;
        }
        for (int i = 0; i < 256; i++) {
            decodeTable[i] = char(-1);
        }
        for (int i = 0; i < 64; i++) {
            int c = encodeB64URLTable[i];
            c = c & 0xFF;
            *(decodeTable + c) = (unsigned char)(i);
        }
        // also, decode the other older crappy type:
        decodeTable['-'] = 62;
        decodeTable['/'] = 63;
    }

    const char * getTable(){
        return encodeB64URLTable;
    }

    // encode the bytes from src[0] to src[srcLen] into dest.
    // dest has a size of dest_max which must be greater than src_len*4/3
    // the number of bytes written is returned.
    int encode(const unsigned char *src, int srcLen, char *dest, int destMax) {
        int srci = 0;
        int dsti = 0;

        while (srci < srcLen) {
            dest[dsti++] = encodeB64URLTable[src[0 + srci] >> 2];

            if (dsti >= destMax) {
                return dsti;
            }
            if (srci + 1 >= srcLen) {
                dest[dsti++] = encodeB64URLTable[((src[0 + srci] & 0x03) << 4) | (0 >> 4)];
                return dsti;
            }

            dest[dsti++] = encodeB64URLTable[((src[0 + srci] & 0x03) << 4) | (src[1 + srci] >> 4)];

            if (dsti >= destMax) {
                return dsti;
            }
            if (srci + 2 >= srcLen) {
                dest[dsti++] = encodeB64URLTable[((src[1 + srci] & 0x0f) << 2) | (0 >> 6)];
                return dsti;
            }

            dest[dsti++] = encodeB64URLTable[((src[1 + srci] & 0x0f) << 2) | (src[2 + srci] >> 6)];

            if (dsti >= destMax) {
                return dsti;
            }

            dest[dsti++] = encodeB64URLTable[src[2 + srci] & 0x3f];

            if (dsti >= destMax) {
                return dsti;
            }
            srci += 3;
        };

        return dsti;
    }

    // Decode the bytes from src[0] to src[srcLen] into dest.
    // dest has a size of destMax which must be greater than srcLen*3/4
    // The number of bytes written is returned.
    // GIGO.
    int decode(const unsigned char *src, int srcLen, char *dest, int destMax) {
        initdecodetable();

        int srci = 0;
        int dsti = 0;

        while (srci < srcLen) {
            if (src[0 + srci] == 0) {
                return dsti;
            }
            if (srci + 1 >= srcLen) {
                return dsti;
            }
            if (src[1 + srci] == 0) {
                return dsti;
            }

            dest[dsti++] = (decodeTable[src[0 + srci]] << 2) | (decodeTable[src[1 + srci]] >> 4);

            if (dsti >= destMax) {
                return dsti;
            }
            if (srci + 2 >= srcLen) {
                return dsti;
            }
            if (src[2 + srci] == 0) {
                return dsti;
            }

            dest[dsti++] = (decodeTable[src[1 + srci]] << 4) | (decodeTable[src[2 + srci]] >> 2);

            if (dsti >= destMax) {
                return dsti;
            }
            if (srci + 3 >= srcLen) {
                return dsti;
            }
            if (src[3 + srci] == 0) {
                return dsti;
            }

            dest[dsti++] = (decodeTable[src[2 + srci]] << 6) | decodeTable[src[3 + srci]];

            if (dsti >= destMax) {
                return dsti;
            }
            srci += 4;
        }
        return dsti;
    }
    bool isB64(char c) {
        return decodeTable[(unsigned char)c] != 0xFF;
    }

} // base64

namespace hex {

    bool isHex(char c) {

        if (c >= 'A' && c <= 'F') {
            return true;
        }
        if (c >= 'a' && c <= 'f') {
            return true;
        }
        if (c >= 'A' && c <= 'F') {
            return true;
        }
        return false;
    }

    unsigned char gethexdigit(unsigned char c) {
        c &= 0x0F;
        if (c <= 9) {
            return char('0' + c);
        }

        return char('a' + c - 10);
    }

    // Encode the bytes from src[0] to src[srcLen] into dest.
    // dest has a size of destMax which must be greater than srcLen*2
    // The number of bytes written is returned.
    int encode(const unsigned char *src, int srcLen, char *dest, int destMax) {
        int srci = 0;
        int dsti = 0;

        while (srci < srcLen) {
            char c = src[srci++];
            dest[dsti++] = gethexdigit(c >> 4);
            if (dsti >= destMax) {
                return dsti;
            }
            dest[dsti++] = gethexdigit(c);
            if (dsti >= destMax) {
                return dsti;
            }
        }
        return dsti;
    }

    unsigned char HexNibbleToNum(char c) {
        unsigned char v;
        if (c >= '0' && c <= '9') {
            v = c - '0';
        } else {
            v = c - 'a' + 10;
        }
        return v;
    }

    // Decode the bytes from src[0] to src[srcLen] into dest. Unless null encountered.
    // dest has a size of destMax which must be greater than srcLen*1/2
    // The number of bytes written is returned.
    int decode(const unsigned char *src, int srcLen, char *dest, int destMax) {
        int srci = 0;
        int dsti = 0;

        while (srci < srcLen) {
            char c1 = src[srci];
            if (c1 == 0) {
                return dsti;
            }
            srci++;
            if (srci >= srcLen) {
                return dsti;
            }
            char c2 = src[srci];
            if (c1 == 0) {
                return dsti;
            }
            srci++;
            if (dsti >= destMax) {
                return dsti;
            }
            dest[dsti++] = (HexNibbleToNum(c1) << 4) + HexNibbleToNum(c2);
        }
        return dsti;
    }

} // hex

namespace base64 {

    // Scan the input and find the longest contigous block of base64
    // compatible chars. If there is more than 47 characters in the range 0..9 or a..f
    // assume it's really hex and decode that.
    // Probability of a 48 char base64 encoding being all hex is (16/64)^48 = 1/(2^96)

    int decodeAll(const unsigned char *src, int srcLen, char *dest, int destMax) {
        int destPos = 0; // return this
        initdecodetable();
        int max1 = 0;
        int max2 = 0;
        bool washex = false;

        int start = -1;
        bool starthex = false;
        int i = 0;
        for (; i < srcLen; i++) {
            if (decodeTable[src[i]] == (unsigned char)(0xFF)) {
                // not b64
                if (start >= 0) {
                    if ((i - start) > (max2 - max1)) {
                        max1 = start;
                        max2 = i;
                        washex = starthex;
                    } else {
                    }
                }
                start = -1;
            } else {
                // b64
                if (start < 0) {
                    start = i;
                    starthex = true;
                } else {
                }
                char c = src[i];
                if (((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')) == false)

                {
                    starthex = false;
                }
            }
        }
        if (start >= 0) {
            if ((i - start) > (max2 - max1)) {
                max1 = start;
                max2 = i;
                washex = starthex;
            } else {
            }
        }

        if (max2 - max1 >= 48 && washex) {
            destPos = hex::decode(src + max1, max2 - max1, dest, destMax);
        } else {
            destPos = base64::decode(src + max1, max2 - max1, dest, destMax);
        }

        return destPos;
    }

} // namespace base64

namespace utf8 {

    // Adapted from libutf8 by Made to Order Software Corp.
    // TODO: make unit tests and then delete this TODO
    // I feel like this is unnecessarily complex as it's really verifying
    // and not just finding the length.
    int DecodeRuneLengthInString(const unsigned char *s, int len) {
        if (len <= 1)
            return 1;
        if (s[0] <= 0x7F) {
            return 1;
        }
        if (s[0] >= 0xC2 && s[0] <= 0xDF // non-overlong 2-byte
            && s[1] >= 0x80 && s[1] <= 0xBF) {
            return 2;
        }
        if (s[0] == 0xE0 // excluding overlongs
            && s[1] >= 0xA0 && s[1] <= 0xBF && s[2] >= 0x80 && s[2] <= 0xBF) {
            return 3;
        }
        if (((0xE1 <= s[0] && s[0] <= 0xEC) || s[0] == 0xEE || s[0] == 0xEF) // straight 3-byte
            && s[1] >= 0x80 && s[1] <= 0xBF && s[2] >= 0x80 && s[2] <= 0xBF) {
            return 3;
        }
        if (s[0] == 0xED // excluding surrogates
            && s[1] >= 0x80 && s[1] <= 0x9F && s[2] >= 0x80 && s[2] <= 0xBF) {
            return 3;
        }
        if (s[0] == 0xF0 // planes 1-3
            && s[1] >= 0x90 && s[1] <= 0xBF && s[2] >= 0x80 && s[2] <= 0xBF && s[3] >= 0x80 && s[3] <= 0xBF) {
            return 4;
        }
        if (s[0] >= 0xF1 && s[0] <= 0xF3 // planes 4-15
            && s[1] >= 0x80 && s[1] <= 0xBF && s[2] >= 0x80 && s[2] <= 0xBF && s[3] >= 0x80 && s[3] <= 0xBF) {
            return 4;
        }
        if (s[0] == 0xF4 // plane 16
            && s[1] >= 0x80 && s[1] <= 0x8F && s[2] >= 0x80 && s[2] <= 0xBF && s[3] >= 0x80 && s[3] <= 0xBF) {
            return 4;
        }
        {
            // not a supported character
            // maybe we should return 0
            return 1;
        }
    }

} // namespace

// Copyright 2022 Alan Tracey Wootton
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
