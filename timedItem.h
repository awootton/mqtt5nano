

#pragma once

namespace mqtt5nano {
    /** TimedItem is used to do thinks like "blink without calling sleep()"
     * There are many explanations of how to accomplish this on the web.
     * I like to do it like this because it's more organized.
     * It's fluffy but hard to get wrong.
     * Here's an example of how to use it:
     *
        todo: atw
     *
     */

    struct TimedItem {
    private:
        void linkToHead();
        TimedItem *nextTimer = nullptr;
        unsigned long nextEventTime = 0;
        bool isWaitingInfinity = false;

    public:
        unsigned long intervalOn = 1000;  // 1 second is the default
        unsigned long intervalOff = 1000; // 1 second is the default
        bool isOn = false;

        unsigned int count = 0;

        TimedItem() {
            linkToHead();
        }
        TimedItem(unsigned long i) {
            linkToHead();
            SetInterval(i);
        }
        // SetInterval sets a period for the callback. For people who don't care about on and off.
        void SetInterval(unsigned long i) {
            SetOnInterval(i);
            SetOffInterval(i);
        }
        // SetOnInterval sets an interval for just the on portion of the cycle.
        // 0 means on forever.
        void SetOnInterval(unsigned long i) {
            // if it's off forever, turn it on
            if (isWaitingInfinity && i != 0) {
                isWaitingInfinity = 0;
                nextEventTime = 0; // will trigger state change next loop
            }
            intervalOn = i;
        }
        // SetOnInterval sets an interval for just the off portion of the cycle.
        // 0 means off forever.
        void SetOffInterval(unsigned long i) {
            // if it's on forever, turn it off
            if (isWaitingInfinity && i != 0) {
                isWaitingInfinity = 0;
                nextEventTime = 0; // will trigger state change next loop
            }
            intervalOff = i;
        }
        virtual void init(){};
        virtual void execute() = 0;

    private:
        void loop(unsigned long now) {
            if (nextEventTime == 0) {
                nextEventTime = now;
            }
            long delta = nextEventTime - now;
            if (delta <= 0) {
                bool wasOn = isOn;
                isOn = !isOn;
                execute();
                unsigned long tmp = isOn ? intervalOn : intervalOff;
                if (tmp == 0) {
                    tmp = (unsigned long)1000 * 60 * 60 * 24 * 365 * 10; // ten years
                    isWaitingInfinity = true;
                }
                nextEventTime += tmp;
                count++;
            }
        }

    public:
        // LoopAll will call the loop of every timer
        static void LoopAll(unsigned long now);
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
