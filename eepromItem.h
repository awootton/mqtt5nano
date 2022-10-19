
#pragma once

#include "mockEEPROM.h"
 
#include "slices.h"

/** EepromItem is a somple utility to declare and use some EEPROM memory
 * that is persistant across reboots.
 * To use it add a reference to it in the header like this:
 * extern EepromItem ssidStash;
 * and then, in the cpp file, declare it like this:
 * EepromItem ssidStash(32, "ssid");
 */ 
namespace mqtt5nano
{
    // we could make an EEDrain and people can just write 
    struct EepromItem
    {
        int size;
        int offset;
        EepromItem *next;
        const char *description;

        EepromItem(int size, const char *description);

        bool read(drain &out)
        {
            for (int i = 0; i < size; i++)
            {
                char c = EEPROM.read(offset + i);
                if (c == 0)
                {
                    break;
                }
                out.writeByte(c);
            }
            return true; // ok
        }

        bool write(slice bytes)
        {
            int i;
            for (i = 0; i < size-1; i++)
            {
                if (i >= bytes.size())
                {
                    break;
                }
                char c = bytes.base[bytes.start + i];
                EEPROM.write(offset + i, c);
            }
            EEPROM.write(offset + i, 0);
            EEPROM.commit();
            return true;// ok
        }
    };

    EepromItem *getEitemHead(); // do we need this?
    // this is for EEPROM.begin(mqtt5nano::getEitemTotal()); which is needed in setup.
    int getEitemTotal(); 

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
