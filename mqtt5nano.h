// #include "Arduino.h"

#pragma once


/** This file includes all the rest of the headers in this package.
 *
 */

#include "knotStream.h" // this does nothing on Arduino. A stub.

// this is the mqtt5 parser and generator.
#include "mqtt5nanoParse.h"

// timedItem is a simple gadget for doing things like blink
// without using sleep.

#include "timedItem.h"

// drags in the help command
#include "streamReader.h"

// #include "commandLine.h"
#include "eepromItem.h"

#include "wiFiCommands.h"
#include "commonCommands.h"

#include "httpServices.h"

#include "mqttCommands.h"

namespace mqtt5nano
{
    extern long latestNowMillis;

    extern MqttCommandClient *mqttClientP;
    
    struct PackageOne
    {
        streamReader serialCommandHandler;
        WiFiHelper wifi;
        WebServer www;

        getUptime up;
        getServed served;

        MqttCommandClient mqttClient;

        void setup(class Stream &serial)
        {
            serial.println("hello knotfree package one");
            serial.println("open a console and type something");
            
            EEPROM.begin(mqtt5nano::getEitemTotal());
            // // serialCommandHandler.setup((char *)readbuffer, (int)sizeof(readbuffer));
        }

        void loop(long now, class Stream &serial)
        {
            serialCommandHandler.loop(serial);
            wifi.loop(now, serial);
            www.loop(now, serial);
            mqttClient.loop(now, serial);
        }
    };
}

//#include "textCmdSerial.h"

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
