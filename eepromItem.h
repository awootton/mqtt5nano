
#pragma once

#include "mockEEPROM.h"

#include "slices.h"

#include "mockStream.h"

/** EepromItem is a somple utility to declare and use some EEPROM memory
 * that is persistant across reboots.
 * To use it add a reference to it in the header like this:
 * extern EepromItem ssidStash;
 * and then, in the cpp file, declare it like this:
 * EepromItem ssidStash(32, "ssid");
 */
namespace mqtt5nano {

    // extern class Stream *globalSerial;

    struct EepromItem *getEitemHead(); // do we need this?
    // this is for EEPROM.begin(mqtt5nano::getEitemTotal()); which is needed in setup.
    int getEitemTotal();

    // initAll will check if they need to be initialized.
    void initAllEeItem();

    // we could make an EEDrain and people can just write
    struct EepromItem {
        int size;
        int offset;
        EepromItem *next;
        const char *description;
        const char *initialValue;
        bool started = false;

        EepromItem(int size, const char *description, const char *initialValue);

        bool empty() {
            char c = EEPROM.read(offset + 0);
            if (c == 0) {
                return true;
            }
            return false;
        }

        bool read(Destination &out) {

            // checkStarted();
            int i;
            for (i = 0; i < size; i++) {
                char c = EEPROM.read(offset + i);
                if (c == 0) {
                    break;
                }
                bool ok = out.writeByte(c);
                if (!ok) {
                    return false;
                }
            }
            return true; // ok TODO: check for drain full
        }

        slice readSlice(ByteDestination d) {
            int startPos = d.buffer.start;
            bool ok = read(d);
            if (!ok) {
                return slice();
            }
            slice result(d.buffer.base, startPos, d.buffer.start);
            return result;
        }

        bool readAll(Destination &out) {

            int i;
            for (i = 0; i < size; i++) {
                char c = EEPROM.read(offset + i);
                // globalSerial->print(c);
                // if (c == 0) {
                //    break;
                //}
                out.writeByte(c);
            }
            return true; // ok
        }

    private:
        void writeone(int i, char c) {
            EEPROM.write(i, c);
            // delay(1);
            // or is is put?
            // EEPROM.put(i, &c);
            // globalSerial->print(i);
        }

    public:
        void checkStarted() {
            if (started) {
                return;
            }
            started = true;
            // globalSerial->print("ee starting ");
            // globalSerial->println(description);

            int i;
            int qcount = 0;
            for (i = 0; i < size; i++) { // read the values once for strlen
                char c = EEPROM.read(offset + i);
                // delay(1);
                if (c == 0) {
                    break;
                }
                if ((c & 0x80) != 0) {
                    qcount++;
                }
            }
            if (qcount * 2 >= size) {
                globalSerial->print("# qcount ");
                globalSerial->println(qcount);
            }
            if (i == size || (qcount * 2 >= size)) { // happens when never inited.
                // we really don't like never inited.
                globalSerial->print("# init ");
                globalSerial->print(description);
                globalSerial->print(" with ");
                globalSerial->println(initialValue);
                // clear it.
                for (i = 0; i < size; i++) {
                    writeone(offset + i, 0);
                }
                // write the default
                write(slice(initialValue));
            }
        }

        bool write(slice bytes) {
            int i;
            // globalSerial->print("# eeprom # ");
            for (i = 0; i < size - 1; i++) {
                if (i >= bytes.size()) {
                    break;
                }
                char c = bytes.base[bytes.start + i];
                writeone(offset + i, c);
                // globalSerial->print(c);
            }
            writeone(offset + i, 0); // null terminated

            bool ok = EEPROM.commit();
            return true; // ok
        }
    };

}

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
