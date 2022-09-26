

#pragma once

namespace knotfree
{
    /** This gadget is used to do stuff link 'blink' without calling sleep()
     * There are many explanations of how to acomplsh this on the web.
     * I like to do it like this because it's more organized.
     * It's fluffy but hard to get wrong. 
     * Here's an example of how to use it:
     * 
        #include <knotfree.h> // include it.

        #define LED_BUILTIN 2 // for ESP32

        struct blinker : knotfree::TimedItem { // define it
            bool isOn = false;
            void init() override {
                SetInterval(1200);// 1.2 second
            }
            void execute() override {
                if ( isOn ){
                    digitalWrite(LED_BUILTIN, LOW);
                } else {
                    digitalWrite(LED_BUILTIN, HIGH);
                }
                isOn = ! isOn;
            }
        };
        blinker myBlinker; // create it

        void setup()
        {
            Serial.begin(115200);
            delay(10);
            Serial.println("hello wifi commands test");
            pinMode(LED_BUILTIN, OUTPUT);
        }

        void loop()
        {
            myBlinker.loop(millis()); // call it
        }
     *
     */

    struct TimedItem
    {
        unsigned long interval = 1000; // 1 second is the default
        unsigned long nextEvent = 0;

        TimedItem() {}
        TimedItem(unsigned long i)
        {
            SetInterval(i);
        }
        void SetInterval(unsigned long i)
        {
            interval = i;
        }
        virtual void init(){};
        virtual void execute() = 0;
        void loop(unsigned long now)
        {
            if (nextEvent == 0)
            {
                nextEvent = now;
            }
            long delta = nextEvent - now;
            if (delta < 0)
            {
                execute();
                nextEvent += interval;
            }
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
