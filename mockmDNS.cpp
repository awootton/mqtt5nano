#include "mockmDNS.h"

#if not defined(ARDUINO)

mockMdns MDNS;

#endif

#if defined(ESP8266)

    const char *hostName(int u) {
        return MDNS.answerHostname(u);
    }
    void removeQuery() {
        MDNS.removeQuery();
    }
    void mdnsUpdate() {
        MDNS.update();
    }

#elif defined(ESP32)

    const char *hostName(int u) {
        return MDNS.hostname(u).c_str();
    }
    void removeQuery() {
        //  MDNS.;
    }
    void mdnsUpdate() {
       // MDNS.update();
    }

#else
    // something else 
    const char *hostName(int u) {
        return MDNS.answerHostname(u);
    }
    void removeQuery() {
        MDNS.removeQuery();
    }
    void mdnsUpdate() {
        MDNS.update();
    }


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


