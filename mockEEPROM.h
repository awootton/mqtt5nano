
#pragma once

#if defined(ARDUINO) and not defined(ARDUINO_ARCH_SAMD)

#if defined(ESP8266)
#include <EEPROM.h>
#elif defined(ESP32)
#include <EEPROM.h>
#elif defined(ARDUINO_SAMD_MKRWIFI1010)
// use flash storage somehow
#else
not 8266 or 32 or samd
#endif

#else


#include "nanoCommon.h"

using namespace std;

// this is a MOCK.
// It's unused irl. TODO: (atw)

struct mockEEPROM {

    // put this in flash if there's no eeprom.
    // if ARDUINO_SAMD_MKRWIFI1010 then use flash storage
    char eeMockBuffer[4096];

    void begin(int amt) {
        // alloc the eeMockBuffer with this amount TODO:
        if (amt > 4096) {
            // cout << "things have ee limitations";
        }
        for (int i = 0; i < sizeof(eeMockBuffer); i++) {
            eeMockBuffer[i] = -1;
        }
    }

    void end() {}

    char read(int offset) {
        return eeMockBuffer[offset];
    }

    void write(int offset, char c) {
        eeMockBuffer[offset] = c;
    }
    bool commit() {
        return true;
    }
};

extern mockEEPROM EEPROM;

#endif

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
