
#pragma once

namespace mqtt5nano
{
    // Slice represents a read only sequence of bytes. The size of the slice, and the underlaying array,
    // are limited to 64k.
    // Some of the methods increment the 'start' as they parse.
    // We pass them around by value most times. They are NOT null terminated.

    // We don't malloc or free or new these ever. They live embedded in structs
    // or as local variables. The underlaying buffer is another story.
    // many times the underlaying buffer is, in fact, a Serial or tcp receive buffer.

    struct sink;  // sink is the not-read-only version of slice. defined below
    //struct fount; // fount is like a slice except the readByte is virtual and not just from a buffer.
    struct drain; // is like a sink except the writeByte is virtual and not from a buffer.

    struct slice
    {
        const char *base;
        unsigned short int start;
        unsigned short int end;

        slice() // returns an empty slice
        {
            base = 0;
        }

        // init with c string
        slice(const char *str)
        {
            base = str;
            int len = 0; // calc strlen(str); I don't want to drag in c libs.
            for (; str[len]; len++)
                ;
            start = 0;
            end = len;
        }
        slice(const char *str, int i1, int i2)
        {
            base = str;
            start = i1;
            end = i2;
        }

        // Init a slice with the sink ptr,start,end
        slice(sink s);

        bool empty()
        {
            if (!base || start >= end)
            {
                return true;
            }
            return false;
        };

        int size()
        {
            if (empty())
            {
                return 0;
            }
            return end - start;
        }
        int length()
        {
            return size();
        }
        static const char *emptyStr; // in the cpp
        const char *charPointer()    // a pointer to the first char. not null terminated
        {
            if (size())
            {
                return base + start;
            }
            return emptyStr; // pointer to 0
        }

        // return the first byte and shrink the size. aka pop.
        unsigned char readByte() // advances start
        {
            if (empty())
            {
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
            do
            {
                if (empty())
                {
                    return -1;
                }
                tmp = readByte();
                i++;
                val = (val << 7) | (tmp & 0x7F);
                if (i == 3)
                {
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
            do
            {
                if (empty())
                {
                    return -1;
                }
                tmp = readByte();
                val |= int(tmp & 0x7F) << (i * 7);
                i++;
                if (i == 3)
                {
                    break;
                }
            } while (tmp >= 128);
            return val;
        };

        // getFixLenInt pops two bytes big-endian and returns an int.
        int getBigFixLenInt() // advances start
        {
            if (empty())
            {
                return 0;
            }
            int val = readByte();
            val = (val << 8) & 0xFF00;
            val |= readByte();
            return val;
        };

        int getLittleFixLenInt() // advances start
        {
            if (empty())
            {
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
            if (len > size())
            {
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
            if (len > size())
            {
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
        bool equals(slice rhs)
        {
            if (length() != rhs.length())
            {
                return false;
            }
            int len = length();
            const char *cp1 = base + start;
            const char *cp2 = rhs.base + rhs.start;
            for (int i = 0; i < len; i++)
            {
                if (cp1[i] != cp2[i])
                {
                    return false;
                }
            }
            return true;
        }

        // equals returns true if this slice has the same bytes as the null terminated str.
        bool equals(const char *str)
        {
            if (empty()) // never ever iterate an empty slice.
            {
                if (str[0] == 0)
                {
                    return true;
                }
                return false;
            }
            int i = start;
            for (; i < end; i++)
            {
                char c = str[i - start];
                if (c == 0)
                {
                    return false;
                }
                if (c != base[i])
                {
                    return false;
                }
            }
            char c = str[i - start];
            if (c != 0)
            { // it needs to be null now.
                return false;
            }
            return true;
        }
        // startsWith returns true if the slice starts with the str
        bool startsWith(const char *str)
        {
            int i = 0;
            for (i = 0; i < size(); i++)
            {
                if (str[i] == 0)
                {
                    return true;
                }
                if (base[start + i] != str[i])
                {
                    return false;
                }
            }
            if (str[i] == 0)
            {
                return true;
            }
            return false;
        }

        // for debugging.
        // Copy out the slice into the buffer.
        char *getCstr(char *buffer, int max); // in the cpp
        // copy the slice into the buffer while expanding into hex.
        char *gethexstr(char *buffer, int max); // in the cpp
        void gethexstr(sink dest);              // defined in the cpp

    }; // slice

    // sink is like a slice except that we can write to it.
    // All the write or put functions you might expect in slice are in here.
    // We write at the start and then increment start until start
    // gets to end and then we're full.
    // Note that start is past the bytes written
    // and the bytes saves are between 0 and start
    struct sink
    {
        char *base;
        unsigned short int start; // write position
        unsigned short int end;

        sink()
        {
            base = 0;
        }
        sink(char *cP, int amt)
        {
            base = cP;
            start = 0;
            end = amt;
        }
        // sink(slice s) // sorry, slice is read only
        int remaining()
        {
            return end - start;
        };
        int amount() // how much has been written
        {
            return start;
        };
        bool full()
        {
            if (start >= end || base == 0)
            {
                return true;
            }
            return false;
        }
        // writeByte copies the char into the buffer and advances the start index. returns ok.
        bool writeByte(char c)
        {
            bool ok = true;
            if (full() == false)
            {
                base[start] = c;
                start++;
                return true;
            }
            return false; // not ok
        }
        // writeBytes calls writeByte returns ok
        bool writeBytes(const char *cP, int amt)
        { // we could just construct a slice and call write. todo
            bool ok = true;
            for (int i = 0; i < amt; i++)
            {
                ok = writeByte(cP[i]);
                if (!ok)
                    return ok;
            }
            return true; // ok
        }
        // returns ok
        bool write(slice s)
        {
            bool ok = true;
            if (s.empty())
            {
                return false; // not ok
            }
            for (int i = s.start; i < s.end; i++)
            {
                ok &= writeByte(s.base[i]);
            }
            return ok;
        };
        bool write(sink s)
        {
            return write(slice(s));
        };
        bool writeFixedLenStr(slice s)
        {
            bool ok = true;
            int len = s.size();
            if (len + 2 > end)
            {
                ok = false;
                return ok;
            }
            writeByte(len >> 8);
            writeByte(len);
            for (int i = 0; i < len; i++)
            {
                writeByte(s.base[s.start + i]);
            }
            end += len;
            return ok;
        }
        // the int needs to be less than 2^21
        // mqtt needs litle endian.
        bool writeBigEndianVarLenInt(int val)
        {
            bool ok = true;
            if (val >= 128)
            {
                if (val > 128 * 128)
                {
                    ok &= writeByte((val >> 14) | 0x80);
                }
                ok &= writeByte((val >> 7) | 0x80);
            }
            ok &= writeByte(val & 0x7F);
            return ok;
        }
        bool writeLittleEndianVarLenInt(int val)
        {
            while (true)
            {
                if (val < 128)
                {
                    writeByte(val);
                    break;
                }
                else
                {
                    writeByte((val & 0x7F) | 0x80);
                }
                val = val >> 7;
            }
            return !full();
        }
        // getWritten return the slice BEFORE the start.  It's what's *been* written.
        slice getWritten()
        {
            slice s;
            s.base = base;
            s.start = 0;
            s.end = start;
            return s;
        }
        void reset()
        {
            start = 0;
        }
    };

    // fount can act as an incoming stream of bytes. It's very much like a slice except
    // In Arduino it's a Stream. There is a StreamFount.
    // the source of the byte stream is virtual. virtual virtual virtual.
    // like a network connection.
    // outside of test it may block.
    // there is an implementation where the bytes come from a slice. See sliceFount
    // struct fount
    // {
    //     virtual unsigned char readByte()
    //     {
    //         return 0;
    //     }
    //     virtual bool empty()
    //     {
    //         return true;
    //     }
    //     int getBigEndianVarLenInt()
    //     {
    //         int val = 0;
    //         int i = 0;
    //         unsigned char tmp;
    //         do
    //         {
    //             tmp = readByte();
    //             i++;
    //             val = (val << 7) | (tmp & 0x7F);
    //             if (i == 3)
    //             {
    //                 break;
    //             }
    //         } while (tmp >= 128);
    //         return val;
    //     };
    //     int getLittleEndianVarLenInt()
    //     {
    //         int val = 0;
    //         int i = 0;
    //         unsigned char tmp;
    //         do
    //         {
    //             tmp = readByte();
    //             val += int(tmp & 0x7F) << (i * 7);
    //             i++;
    //             if (i == 3)
    //             {
    //                 break;
    //             }
    //         } while (tmp >= 128);
    //         return val;
    //     };
    // };

    // drain can act as a place to send a stream of bytes.
    // It's a lot like a sink except the desination is virtual and not just a buffer.
    // in Arduino it's a Stream. There is a StreamDrain.
    // Like a network connection.
    // Return true when things are failed.
    // There is an implementation where the drain is a sink. SinkDrain exists. and CoutDrain
    // there's  NetDrain in tests.
    struct drain
    {
        virtual bool writeByte(char c) = 0; // returns ok

        // write the bytes of s down the drain. returns ok
        bool write(slice s)
        {
            if (s.empty())
            {
                return false;
            }
            for (int i = 0; i < s.size(); i++)
            {
                bool ok = writeByte(s.base[s.start + i]);
                if (!ok)
                {
                    return ok;
                }
            }
            return true;
        }
        bool write(const char *cP) // c string
        {
            for (; *cP != 0; cP++)
            {
                writeByte(*cP);
            }
            bool ok = true;
            return ok;
        }
        void localWriteHelper( int i ){
            if ( i == 0 ){
                return;
            }
            localWriteHelper(i/10);
            int tmp = i%10;
            writeByte('0' + tmp);
        }
        bool writeInt(int i)
        {
            if ( i == 0 ){
                writeByte('0');
                return true;
            }
            if ( i < 0 ){
               writeByte('-'); 
               i = -1;
            }
            localWriteHelper(i);
           return true;
        }

        bool writeBytes(const char *cP, int amt)
        { // we could just construct a slice and call write. todo
            bool ok = true;
            for (int i = 0; i < amt; i++)
            {
                ok = writeByte(cP[i]);
                if (!ok)
                    return ok;
            }
            return ok;
        }

        // writeFixedLenStr writes a 2 byte length big endian
        // and then the string bytes
        bool writeFixedLenStr(slice str)
        {
            int len = str.size();
            bool ok = writeByte(len >> 8);
            if (!ok)
            {
                return ok;
            }
            ok = writeByte(len);
            if (!ok)
            {
                return ok;
            }
            for (int i = 0; i < len; i++)
            {
                ok = writeByte(str.base[str.start + i]);
                if (!ok)
                {
                    return ok;
                }
            }
            return false;
        }
    };
    // a fount that is a slice.
    // struct SliceFount : fount
    // {
    //     slice src;
    //     SliceFount(slice src) : src(src) {}
    //     SliceFount() {}
    //     virtual unsigned char readByte()
    //     {
    //         return src.readByte();
    //     }
    //     virtual bool empty()
    //     {
    //         return src.empty();
    //     }
    // };
    // a drain that is a sink.
    // writes to this drain are written to the drain's buffer.
    // struct SinkDrain : drain
    // {
    //     sink dest;
    //     // returns ok
    //     bool writeByte(char c) override
    //     {
    //         return dest.writeByte(c);
    //     };
    // };

    // sliceResult is for when we want to return slice,char
    // which requires a struct in C++
    struct sliceResult
    {
        slice s;
        char error;
        sliceResult()
        {
            error = 0;
        }
    };
    // intResult is for when we want to return int,char
    struct intResult
    {
        int i;
        unsigned char error;
        intResult()
        {
            error = 0;
        }
    };

    struct SinkDrain : drain // for the tests output to a buffer
    {
        sink buffer;
        SinkDrain() {}
        SinkDrain(char *cP, int len)
        {
            buffer.base = cP;
            buffer.start = 0;
            buffer.end = len - 1;
        }

        bool writeByte(char c) override
        {
            return buffer.writeByte(c);
        };

        void reset()
        {
            buffer.reset();
        }
        slice getWritten()
        {
            return buffer.getWritten();
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
