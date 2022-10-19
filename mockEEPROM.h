
#pragma once

#if defined(ARDUINO)
#include <EEPROM.h>
#else
 
// this is a MOCK.
// It's unused and untested. TODO: (atw)

struct mockEEPROM
{
    char eeMockBuffer[4096];

    void begin( int amt){
        // alloc the eeMockBuffer with this amount TODO:
    }

    char read(int offset)
    {
        return eeMockBuffer[offset];
    }

    void write(int offset, char c)
    {
        eeMockBuffer[offset] = c;
    }
    void commit(){
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
