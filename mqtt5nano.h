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

#include "mockStream.h" // this does nothing on Arduino. A stub.
#include "nanoCommon.h"

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

        void setMillisFunction(int (*anotherGetMillis)()) { // optional
            getMillis = anotherGetMillis;
        }

        void setup(class Stream &serial) {
            globalSerial = &serial;              // deprecate
            serialDestination.setStream(serial); // use serialDestination.

            serialDestination.println("# hello nano package one");
            serialDestination.println("# type 'help'");

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
            globalSerial = &serial;              // deprecate
            serialDestination.setStream(serial); // use serialDestination.

            latestNowMillis = now;

            serialCommandHandler.loop(now, serial);

            // int newtime = getMillis();
            // if (newtime - latestNowMillis > 1000) {
            //     serialDestination.print("# serialCommandHandler is slow ");
            // }
            // latestNowMillis = newtime;

            wifi.loop(now, serial);

            // newtime = getMillis();
            // if (newtime - latestNowMillis > 1000) {
            //     serialDestination.print("# wifi.loop is slow ");
            // }
            // latestNowMillis = newtime;

 // FIXME: someday get local mode to play nice with the mqtt tcp sockets and esp32
            #ifndef ESP32
            www.loop(now, serial);
            #endif

            // newtime = getMillis();
            // if (newtime - latestNowMillis > 1000) {
            //     serialDestination.print("# www.loop is slow ");
            // }
            // latestNowMillis = newtime;

            mqttClient.loop(now, serial);

            // newtime = getMillis();
            // if (newtime - latestNowMillis > 1000) {
            //     serialDestination.print("# mqttClient.loop is slow ");
            // }
            // now = latestNowMillis = newtime;

            TimedItem::LoopAll((unsigned long)now);

            // newtime = getMillis();
            // if (newtime - latestNowMillis > 1000) {
            //     serialDestination.print("# LoopAll is slow ");
            // }
            // now = latestNowMillis = newtime;

            delay(1); // save power
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
