#pragma once

namespace mqtt5nano {

    // Slice represents a read only sequence of bytes. The size of the slice, and the underlaying array,
    // are limited to 64k.
    // Some of the methods increment the 'start' as they parse.
    // We pass them around by value most times. They are NOT null terminated.

    // We don't malloc or free or new these ever. They live embedded in structs
    // or as local variables. The underlaying buffer is another story.
    // Many times the underlaying buffer is, in fact, a Serial or tcp receive buffer. And we wish to parse it in place.

    // ByteCollector is the not-read-only version of slice.
    // 'start' is past the last byte written and 'end' is the limit of the array.
    struct ByteCollector;

    // Destination is like a ByteCollector except the writeByte is virtual. Bytes are written to a ByteCollector or to a stream or cout etc.
    struct Destination;

    struct slice {

        const char *base;
        unsigned short int start;
        unsigned short int end;

        slice() // returns an empty slice
        {
            base = 0;
        }

        // init with c string
        slice(const char *str) {
            base = str;
            int len = 0; // calc strlen(str); I don't want to drag in c libs.
            for (; str[len]; len++)
                ;
            start = 0;
            end = len;
        }

        // manual init
        slice(const char *str, int i1, int i2) {
            base = str;
            start = i1;
            end = i2;
        }

        // Init a slice with the ByteCollector ptr,start,end
        slice(ByteCollector s);

        bool empty() {
            if (!base || start >= end) {
                return true;
            }
            return false;
        };

        int size() {
            if (empty()) {
                return 0;
            }
            return end - start;
        }

        int length() {
            return size();
        }

        static const char *emptyStr; // in the cpp

        const char *charPointer()    // a pointer to the first char. not null terminated
        {
            if (size()) {
                return base + start;
            }
            return emptyStr; // pointer to 0
        }

        // return the first byte and shrink the size. aka pop.
        unsigned char readByte() // advances start
        {
            if (empty()) {
                return 0;
            }
            return base[start++];
        };

        // getBigEndianVarLenInt will parse a variable length int where it's big-endian and only the last, least
        // significant, byte is <128. So, it's going to be 7 usable bits per byte.
        // 3 will be enough here.
        int getBigEndianVarLenInt() // advances start
        {
            int val = 0;
            int i = 0;
            unsigned char tmp;
            do {
                if (empty()) {
                    return -1;
                }
                tmp = readByte();
                i++;
                val = (val << 7) | (tmp & 0x7F);
                if (i == 3) {
                    break;
                }
            } while (tmp >= 128);
            return val;
        };

        int getLittleEndianVarLenInt() // advances start
        {
            int val = 0;
            int i = 0;
            unsigned char tmp;
            do {
                if (empty()) {
                    return -1;
                }
                tmp = readByte();
                val |= int(tmp & 0x7F) << (i * 7);
                i++;
                if (i == 3) {
                    break;
                }
            } while (tmp >= 128);
            return val;
        };

        // getFixLenInt pops two bytes big-endian and returns an int.
        int getBigFixLenInt() // advances start
        {
            if (empty()) {
                return 0;
            }
            int val = readByte();
            val = (val << 8) & 0xFF00;
            val |= readByte();
            return val;
        };

        int getLittleFixLenInt() // advances start
        {
            if (empty()) {
                return 0;
            }
            int val = readByte();
            val |= (readByte() << 8);
            return val;
        };

        // getLittleFixedLenString reads a 2 byte length and then that many chars.
        // little-endian
        // unless there are not enough then there's a silent fail.
        slice getLittleFixedLenString() // advances start
        {
            slice result;
            int len = getLittleFixLenInt();
            if (len > size()) {
                len = size(); // never go past end
            }
            result.base = base;
            result.start = start;
            result.end = start + len;
            start = result.end;
            return result;
        };

        // getBigFixedLenString reads a var len int and then that many chars
        // return a slice
        slice getBigFixedLenString() // advances start
        {
            slice result;
            int len = getBigFixLenInt();
            if (len > size()) {
                len = size(); // never go past end
            }
            result.base = base;
            result.start = start;
            result.end = start + len;
            start = result.end;
            return result;
        };
        // equals reurns true if the bytes match, Not the same as
        // if the slices match.
        bool equals(slice rhs) {
            if (length() != rhs.length()) {
                return false;
            }
            int len = length();
            const char *cp1 = base + start;
            const char *cp2 = rhs.base + rhs.start;
            for (int i = 0; i < len; i++) {
                if (cp1[i] != cp2[i]) {
                    return false;
                }
            }
            return true;
        }

        // equals returns true if this slice has the same bytes as the null terminated str.
        bool equals(const char *str) {
            if (empty()) // never ever iterate an empty slice.
            {
                if (str[0] == 0) {
                    return true;
                }
                return false;
            }
            int i = start;
            for (; i < end; i++) {
                char c = str[i - start];
                if (c == 0) {
                    return false;
                }
                if (c != base[i]) {
                    return false;
                }
            }
            char c = str[i - start];
            if (c != 0) { // it needs to be null now.
                return false;
            }
            return true;
        }

        // startsWith returns true if the slice starts with the str
        bool startsWith(const char *str) {
            int i = 0;
            for (i = 0; i < size(); i++) {
                if (str[i] == 0) {
                    return true;
                }
                if (base[start + i] != str[i]) {
                    return false;
                }
            }
            if (str[i] == 0) {
                return true;
            }
            return false;
        }

        bool contains(const char *str) {
            if (str[0] == 0) {
                return false;
            }
            int i = 0;
            for (i = 0; i < size(); i++) {      
                if (base[start + i] == str[0]) {
                    int j = 1;
                    for (; str[j]!=0; j++) {
                        if (base[start + i + j] != str[j]) {
                            break;
                        }
                    }
                    if (str[j] == 0) {
                        return true;
                    }
                }
            }
            return false;
        }

        // indexOf returns the index of the first occurance of c in the slice.  
        int indexOf(char c) {
            int i = 0;
            for (i = 0; i < size(); i++) {
                if (base[start + i] == c) {
                    return i;
                }
            }
            return -1;
        }

        // startsWith returns true if the slice starts with the str
        bool startsWith(slice s) {
            int i = 0;
            for (i = 0; i < size(); i++) {
                if (s.empty()) {
                    return true;
                }
                if (base[start + i] != s.base[s.start]) {
                    return false;
                }
                s.start++;
            }
            if (s.empty()) {
                return true;
            }
            return false;
        }

        // toLong returns the slice parsed as base 10.
        long toLong() {
            long val = 0;
            for (int i = start; i < end; i++) {
                val = val * 10 + base[i] - '0';
            }
            return val;
        }

        // b64Decode decodes the base64 encoded slice into the buffer.
        slice b64Decode(ByteCollector *buffer);

        // b64Encode encodes the slice into the buffer.
        slice b64Encode(ByteCollector *buffer);

        // copy out the slice into the buffer.
        char *copy(char *buffer, int max);

        // Copy out the slice into the array. Null terminate it.
        char *getCstr(char *buffer, int max);

        // copy the slice into the array while expanding into hex.
        char *gethexstr(char *buffer, int max);

        // genarate hex from the slice into the ByteCollector.
        void gethexstr(ByteCollector dest);

    }; // slice

    // ByteCollector is like a slice except that we can write to it.
    // All the write or put functions you might expect in slice are in here.
    // We write at the start and then increment start until start
    // gets to end and then we're full.
    // Note that start is past the bytes written
    // and the bytes saved are between 0 and start
    struct ByteCollector {
        char *base;
        unsigned short int start; // write position
        unsigned short int end;

        ByteCollector() {
            base = 0;
        }
        ByteCollector(char *cP, int amt) {
            base = cP;
            start = 0;
            end = amt;
        }

        int remaining() {
            return end - start;
        };

        int amount() // how much has been written
        {
            return start;
        };

        bool full() {
            if (start >= end || base == 0) {
                return true;
            }
            return false;
        }

        // writeByte copies the char into the buffer and advances the start index. returns ok.
        bool writeByte(char c) {
            bool ok = true;
            if (full() == false) {
                base[start] = c;
                start++;
                return true;
            }
            return false; // not ok
        }

        // writeBytes calls writeByte returns ok
        bool writeBytes(const char *cP, int amt) { // we could just construct a slice and call write. todo
            bool ok = true;
            for (int i = 0; i < amt; i++) {
                ok = writeByte(cP[i]);
                if (!ok)
                    return ok;
            }
            return true; // ok
        }

        // returns ok
        bool write(slice s) {
            bool ok = true;
            if (s.empty()) {
                return false; // not ok
            }
            for (int i = s.start; i < s.end; i++) {
                ok &= writeByte(s.base[i]);
            }
            return ok;
        };

        bool write(ByteCollector s) {
            return write(slice(s));
        };

        bool writeFixedLenStr(slice s) {
            bool ok = true;
            int len = s.size();
            if (len + 2 > end) {
                ok = false;
                return ok;
            }
            writeByte(len >> 8);
            writeByte(len);
            for (int i = 0; i < len; i++) {
                writeByte(s.base[s.start + i]);
            }
            end += len;
            return ok;
        }

        // the int needs to be less than 2^21
        // mqtt needs litle endian.
        bool writeBigEndianVarLenInt(int val) {
            bool ok = true;
            if (val >= 128) {
                if (val > 128 * 128) {
                    ok &= writeByte((val >> 14) | 0x80);
                }
                ok &= writeByte((val >> 7) | 0x80);
            }
            ok &= writeByte(val & 0x7F);
            return ok;
        }

        bool writeLittleEndianVarLenInt(int val) {
            while (true) {
                if (val < 128) {
                    writeByte(val);
                    break;
                } else {
                    writeByte((val & 0x7F) | 0x80);
                }
                val = val >> 7;
            }
            return !full();
        }

        // getWritten return the slice BEFORE the start.  It's what's *been* written.
        slice getWritten() {
            slice s;
            s.base = base;
            s.start = 0;
            s.end = start;
            return s;
        }
        void reset() {
            start = 0;
        }
    };

    // Destination can act as a place to send a stream of bytes.
    // It's a lot like a ByteCollector except the desination is virtual and not just a buffer.
    // in Arduino it's a Stream. There is a StreamDestination class for that.
    // Return false when things are failed. Typically because an underlaying buffer is full or a connection is lost.
    // There is an implementation where the Destination is a ByteCollector 'ByteDestination'.

    struct Destination {
        virtual bool writeByte(char c) = 0; // returns ok

        // write the bytes of s to the destination. returns ok
        bool write(slice s) {
            if (s.empty()) { // in case the buffer is nullptr
                return true;
            }
            for (int i = 0; i < s.size(); i++) {
                bool ok = writeByte(s.base[s.start + i]);
                if (!ok) {
                    return ok;
                }
            }
            return true;
        }
        bool writeCstr(const char *cP) // c string
        {
            for (; *cP != 0; cP++) {
                writeByte(*cP);
            }
            bool ok = true;
            return ok;
        }
        bool print(const char *cP) // c string
        {
            for (; *cP != 0; cP++) {
                writeByte(*cP);
            }
            bool ok = true;
            return ok;
        }
        
        bool println(const char *cP) // c string
        {
            print(cP);
            return print("\n");;
        }
        
        bool print(slice s) {
            return write(s);
        }
        bool print(slice s, slice s2) {
            write(s);
            return write(s2);
        }
        bool print(slice s, slice s2, slice s3) {
            write(s);
            write(s2);
            return write(s3);
        }
        bool print(slice s, slice s2, slice s3, slice s4) {
            write(s);
            write(s2);
            write(s3);
            return write(s4);
        }
        bool print(slice s, slice s2, slice s3, slice s4, slice s5) {
            write(s);
            write(s2);
            write(s3);
            write(s4);
            return write(s5);
        }

        bool print(int n) // c string
        {
            return writeInt(n);
        }

    private:
        void localWriteHelper(int i) {
            if (i == 0) {
                return;
            }
            localWriteHelper(i / 10);
            int tmp = i % 10;
            writeByte('0' + tmp);
        }

    public:
        bool writeInt(int i) {
            if (i == 0) {
                writeByte('0');
                return true;
            }
            if (i < 0) {
                writeByte('-');
                i = -1;
            }
            localWriteHelper(i);
            return true;
        }

        bool writeBytes(const char *cP, int amt) { // we could just construct a slice and call write. todo
            bool ok = true;
            for (int i = 0; i < amt; i++) {
                ok = writeByte(cP[i]);
                if (!ok)
                    return ok;
            }
            return ok;
        }

        // writeFixedLenStr writes a 2 byte length big endian
        // and then the string bytes
        bool writeFixedLenStr(slice str) {
            int len = str.size();
            bool ok = writeByte(len >> 8);
            if (!ok) {
                return ok;
            }
            ok = writeByte(len);
            if (!ok) {
                return ok;
            }
            for (int i = 0; i < len; i++) {
                ok = writeByte(str.base[str.start + i]);
                if (!ok) {
                    return ok;
                }
            }
            return true;
        }
    };

    // sliceResult is for when we want to return slice,char
    // which requires a struct in C++
    struct sliceResult {
        slice s;
        char error;
        sliceResult() {
            error = 0;
        }
    };
    // intResult is for when we want to return int,char
    struct intResult {
        int i;
        unsigned char error;
        intResult() {
            error = 0;
        }
    };

    struct ByteDestination : Destination // for those that output to a buffer
    {
        ByteCollector buffer;
        ByteDestination() {}
        ByteDestination(char *cP, int len) {
            buffer.base = cP;
            buffer.start = 0;
            buffer.end = len - 1;
        }

        bool writeByte(char c) override {
            return buffer.writeByte(c);
        };

        void reset() {
            buffer.reset();
        }
        slice getWritten() {
            return buffer.getWritten();
        }
    };

    struct VoidDestination : Destination // for those that output to space
    {
        VoidDestination() {}
        int count = 0;
        bool writeByte(char c) override {
            count++;
            return true;
        };
        void reset() {
            count = 0;
        }
        slice getWritten() {
            return slice();
        }
    };

} // namespace mqtt5nano

// Copyright 2020 Alan Tracey Wootton
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
