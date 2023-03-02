

#pragma once

const char *hostName(int i);
void removeQuery();
void mdnsUpdate();

#if defined(ARDUINO)

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <ESPmDNS.h>
#elif defined(ARDUINO_SAMD_MKRWIFI1010)
#include <WiFiNINA.h> // what is mdns on this?
see https://www.arduinolibraries.info/libraries/mdns_generic 
and https://github.com/arduino-libraries/ArduinoMDNS/blob/master/examples/WiFi/WiFiResolvingHostNames/WiFiResolvingHostNames.ino
or https://forum.arduino.cc/t/problem-when-change-library-from-wifi101-h-to-wifinina-h/579142
#endif

#else

// This is a mock. The real one is in #include <ESPmDNS.h>
// example:
// if (MDNS.begin("esp32"))
//   {
//     MDNS.setInstanceName("count server demo");
//     MDNS.addService("http", "tcp", 80);
//     MDNS.addServiceTxt("http","tcp","counter","true");
//   }

struct mockMdns {

    void update() {}

    bool begin(const char *hostname) {
        return true;
    }
    void setInstanceName(const char *name) {
    }
    bool addService(const char *str1, const char *str2, int port) {
        return true;
    }
    bool addServiceTxt(const char *str1, const char *str2, const char *str3, const char *str4) {
        return true;
    }

    int queryService(const char *p, const char *s) { // http", "tcp");
        return 3;
    }

    const char *names[4] = {
        "XFINITY",
        "DccWiFi",
        "woot2",
        "Lovett2"};

    const char *answerHostname(int u) {
        if (u >= 0 && u < 4) {
            return names[u];
        }
        return "";
    }

    void removeQuery() {
    }
};

extern mockMdns MDNS;

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
