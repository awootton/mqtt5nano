

#include "timedItem.h"

namespace mqtt5nano {
    TimedItem *allTimedItems = nullptr;

    void TimedItem::linkToHead() {
        this->nextTimer = allTimedItems;
        allTimedItems = this;
    }

    void TimedItem::LoopAll(unsigned long now) {
        TimedItem *ti = allTimedItems;
        while (ti != nullptr) {
            ti->loop(now);
            ti = ti->nextTimer;
        }
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


// yo , delete me
//  #if defined(ARDUINO)
//  unsigned long GetTime()
//  {
//      return millis();
//  }
//  #else

// // This is a perfect example of the kind of nasty old school C++
// // I'm tryng to keep y'all from having to see. I'm so sorry.
// // this is the standard C++ version of millis()
// #include <chrono>

// unsigned int millis()
// {
//     using namespace std::chrono;
//     return static_cast<uint32_t>(duration_cast<milliseconds>(
//                                      system_clock::now().time_since_epoch())
//                                      .count());
// }

// #endif