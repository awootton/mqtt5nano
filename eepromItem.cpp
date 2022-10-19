
#include "eepromItem.h"

namespace mqtt5nano
{
    EepromItem *eehead;

    int totalSize = 0;

    EepromItem::EepromItem(int size, const char *description) : size(size), description(description)
    {   
        offset = totalSize;
        totalSize += size;
        EepromItem * nxt = eehead;
        eehead = this;
        this->next = nxt;
    }

    EepromItem *getEitemHead(){
        return eehead;
    }
    int getEitemTotal(){
        return totalSize;
    }

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
