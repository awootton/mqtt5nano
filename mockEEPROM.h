
#pragma once


#if defined(ARDUINO)

//extern class Stream *globalSerial;
//#define DEBUGV(...) globalSerial->println("oops")

// #define DEBUGV(fmt, ...) ::printf((PGM_P)PSTR(fmt), ##__VA_ARGS__)

#if defined(ESP8266)
#include <EEPROM.h>
#elif defined(ESP32)
#include <EEPROM.h>
#else
#error "This ain't a ESP8266 or ESP32, buddy!" fixme
#endif

#else

#include <iostream>
#include <string>

#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>
#include "nanoCommon.h"

using namespace std;

// this is a MOCK.
// It's unused irl. TODO: (atw)

struct mockEEPROM {
    char eeMockBuffer[4096];

    void begin(int amt) {
        // alloc the eeMockBuffer with this amount TODO:
        if ( amt > 4096 ){
            cout << "things have ee limitations";
        }
        for ( int i = 0; i < sizeof(eeMockBuffer); i ++ ){
            eeMockBuffer[i] = -1;
        }
    }

    void end(){}

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
