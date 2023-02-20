//

#pragma once

#if defined(ARDUINO)
#include "Arduino.h"
#else
extern void delay(int);
#endif

//

/** This file includes all the rest of the headers in this package.
 *
 */

#include "nanoCommon.h"
#include "mockStream.h" // this does nothing on Arduino. A stub.

// this is the mqtt5 parser and generator.
#include "mqtt5nanoParse.h"
#include "timedItem.h"
// drags in the help command
#include "commandLine.h"
#include "commonCommands.h"
#include "eepromItem.h"
#include "httpServices.h"
#include "mqttCommands.h"
#include "nanoCrypto.h"
#include "nanobase64.h"
// #include "setupWizard.h"
#include "streamReader.h"
#include "wiFiCommands.h"


namespace mqtt5nano {

   // extern class Stream *globalSerial;

    extern EepromItem *eehead;

    struct PackageOne {

        streamReader serialCommandHandler;

        MqttCommandClient mqttClient;
        WebServer www;
        WiFiHelper wifi;

        getUptime up;
        getServed served;
        getVersion version;
        getUnixTimeCmd gettime;

        void setup(class Stream &serial) {
            globalSerial = &serial;
            serial.println("# hello nano package one");
            serial.println("# type 'help'");

            moreScrambled(WiFi.macAddress().c_str());
            moreScrambled(54321);

            int eesize = mqtt5nano::getEitemTotal();
            eesize = (eesize + 3) & 0xFFFFFFFC;
            serial.print("# EEsize ");
            serial.println(eesize);

            EEPROM.begin(eesize);

            char defaultShortName[8]; // = "thing-X";
            char defaultLongName[28]; // = "thing-XXXXXXXXXXXX";
            getRandomString(defaultShortName, sizeof(defaultShortName) - 1);
            getRandomString(defaultLongName, sizeof(defaultLongName) - 1);
            defaultShortName[sizeof(defaultShortName) - 1] = 0;
            defaultLongName[sizeof(defaultLongName) - 1] = 0;

            memcpy(defaultShortName, "thing-", 6);
            memcpy(defaultLongName, "thing-", 6);
            hostStash.initialValue = defaultShortName;
            topicStash.initialValue = defaultLongName;

            initAllEeItem(); // calls checkStarted
            hostStash.initialValue = "";
            topicStash.initialValue = "";

            EEPROM.commit();

            serial.println("# eeprom done");
            // pushWizard(makeGetWifi());
        }

        void loop(long now, class Stream &serial) {
            globalSerial = &serial;
            latestNowMillis = now;

            // serial.println("serialCommandHandler");
            serialCommandHandler.loop(now, serial);

            // serial.println("wifi.loop");
            wifi.loop(now, serial);

            // serial.println("www.loop");
            www.loop(now, serial);

            // serial.println("mqttClient.loop");
            mqttClient.loop(now, serial);

            // serial.println("TimedItem::LoopAll");
            TimedItem::LoopAll((unsigned long)now);
            delay(1);
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
