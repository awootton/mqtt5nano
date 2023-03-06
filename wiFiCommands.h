
#pragma once

#include "nanoCommon.h"
#include "commandLine.h"
#include "eepromItem.h"
#include "mockStream.h"
#include "timedItem.h"

#include "mockWiFi.h"
#include "mockmDNS.h"

#if defined(ARDUINO)
#else
#define PROGMEM
#endif

namespace mqtt5nano {

    extern bool connected;
    extern EepromItem ssidStash; //(32, "ssid");
    extern EepromItem passStash; // (48, "wifi pass");
    extern EepromItem hostStash; // 16 aka short name
    extern int conectCountdown;
    extern bool did_mDns;

    struct reconnecter : TimedItem {

        void execute() override {
            if (WiFi.status() != WL_CONNECTED) {
                if (conectCountdown > 0) {
                    conectCountdown--;
                    return;
                }
                connected = false;
                char ssid[ssidStash.size];
                memset(ssid, 0, sizeof(ssid));
                ByteDestination ssiddrain(ssid, sizeof(ssid));
                ssidStash.read(ssiddrain);
                if (strlen(ssid) == 0) {
                    return;
                }
                char pass[passStash.size];
                memset(pass, 0, sizeof(pass));
                ByteDestination passdrain(pass, sizeof(pass));
                passStash.read(passdrain);
                if (strlen(pass) == 0) {
                    return;
                }
                WiFi.mode(WIFI_STA);
                serialDestination.print(slice("# WiFi connecting "),ssid,"\n");
                
                WiFi.begin(ssid, pass);
                conectCountdown = 60;
                // serialDestination.println("# WiFi.begin");
                SetInterval(500);
            } else {
                SetInterval(5000);
                if (!connected) {
                    serialDestination.println("# WiFi connected");
                }
                connected = true;

                if (did_mDns == false) {
                    serialDestination.println("# init mDns");
                    char host[32];
                    const char *hostP = host;
                    memset(host, 0, sizeof(host));
                    ByteDestination tmpdrain(host, sizeof(host));
                    hostStash.read(tmpdrain);
                    serialDestination.print("# start mDns ",hostP,"\n");
                   #if not defined(ARDUINO_SAMD_MKRWIFI1010)
                    if (MDNS.begin(hostP)) {
                        // MDNS.setInstanceName("mqtt5nano command server");
                        MDNS.addService("http", "tcp", 80);
                        // MDNS.addServiceTxt("http","tcp");
                        // MDNS.addServiceTxt("mqtt","tcp"); // todo:
                        did_mDns = true;
                        serialDestination.println("# did_mDns");
                    } else {
                        serialDestination.println("# ERROR did_mDns");
                    }
                    #endif // FIXME: what is mDNS on ARDUINO_SAMD_MKRWIFI1010 ? 
                }
            }
        }
    };

    struct WiFiHelper {
        reconnecter timer;

        WiFiHelper() {
            timer.SetInterval(5000); // 
        }
        void loop(long now, class Stream &s) {
        }
    };

    void writeStarredPass( EepromItem &stash , Destination &out );
    
    
    struct ssidGet : Command {
        void init() override {
            name = "get WiFi";
            description = "WiFi name";
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {
            out.write("WiFi is: ");
            ssidStash.read(out);
        }
    };

    struct ssidSet : Command {
        void init() override {
            name = "set wifi";
            description = "set WiFi name";
            argumentCount = 1;
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {
            if (args[0].empty()) {
                out.write("ERROR expected a value");
                return;
            }
            ssidStash.write(args[0]);
            out.write("ok: ");
            ssidStash.read(out);
            conectCountdown = 0; // retry now
        }
    };

    struct passGet : Command {
        void init() override {
            name = "get pass";
            description = "WiFi password";
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {
            out.write("pass is: ");
            writeStarredPass(passStash,out);
        }
    };

    struct passSet : Command {
        void init() override {
            name = "set pass";
            description = "set WiFi pass";
            argumentCount = 1;
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {
            if (args[0].empty()) {
                out.write("ERROR expected a value");
                return;
            }
            passStash.write(args[0]);
            out.write("ok: ");
            writeStarredPass(passStash,out);
            conectCountdown = 0; // retry now
        }
    };

    struct hostGet : Command {
        void init() override {
            name = "get short name";
            description = "name on local net.🔓";
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {
            // out.write("short name is: ");
            hostStash.read(out);
        }
    };

    struct wifiStatus : Command {
        void init() override {
            name = "status";
            description = "WiFi status";
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {
            out.write("wifi-connected:");
            out.write(connected ? "true" : "false");
            out.write(" MDNS:");
            out.write(did_mDns ? "true" : "false");
            out.write(" ssid:");
            ssidStash.read(out);
            out.write(" pass:");
            writeStarredPass(passStash,out);
            out.write(" short-name:");
            hostStash.read(out);
            out.write(" ip-addr:");
            IPAddress addr = WiFi.localIP();
            out.writeInt(addr[0]); // eew
            out.write(".");
            out.writeInt(addr[1]);
            out.write(".");
            out.writeInt(addr[2]);
            out.write(".");
            out.writeInt(addr[3]);
            out.write("\n");
        }
    };

    // 83 bytes PROGMEM.  What's the secret to using PROGMEM ?
    // it's a PNG of a green square.
    static const unsigned char PROGMEM png[] = {137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68,
                                        82, 0, 0, 0, 16, 0, 0, 0, 16, 8, 6, 0, 0, 0, 31, 243,
                                        255, 97, 0, 0, 0, 26, 73, 68, 65, 84, 120, 218, 99, 84, 106, 209,
                                        255, 207, 64, 1, 96, 28, 53, 96, 212, 128, 81, 3, 134, 139, 1, 0,
                                        239, 170, 29, 81, 139, 188, 27, 125, 0, 0, 0, 0, 73, 69, 78, 68,
                                        174, 66, 96, 130};

    struct favIcon : Command {
        void init() override {
            name = "favicon.ico";
            description = "shortest png in the world🔓";
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {

            char buff[sizeof(png)];
            memcpy_P(buff,png,sizeof(buff));

            for (int i = 0; i < sizeof(png); i++) {
                out.writeByte((const char)(buff[i]));
            }
        }
    };

#if not defined(ARDUINO_SAMD_MKRWIFI1010) // fixme for MKR1010
    // do we really need this command?
    struct ssidListGet : Command {
        void init() override {
            name = "get wifi list";
            description = "list of local wifi nets";
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {

            WiFi.mode(WIFI_STA);
            WiFi.disconnect();
            connected = false;
            delay(100);

            int n = WiFi.scanNetworks();
            // out.write("scan done");
            if (n == 0) {
                delay(1000);
                for ( int i = 0; i < 5; i++ ) {
                    out.write("no networks found");
                }
            } else {
                // Serial.print(n);
                // Serial.println(" networks found");
                for (int i = 0; i < n; ++i) {
                    // Print SSID and RSSI for each network found
                    out.writeInt(i + 1);
                    out.write(": ");
                    out.write(WiFi.SSID(i).c_str());
                    out.write(" (");
                    out.writeInt(WiFi.RSSI(i));
                    out.write(")");
                    out.write("\n");
                    delay(10);
                }
            }
            WiFi.mode(WIFI_STA);
        }
    };
#endif

    struct peersListGet : Command {
        void init() override {
            name = "get local peers";
            description = "list http servers on local net.";
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {

            uint32_t u32AnswerCount = MDNS.queryService("http", "tcp");
            for (uint32_t u = 0; u < u32AnswerCount; ++u) {
                const char *pHostname = hostName(u);
                out.writeInt(u);
                out.write(") ");
                out.write(pHostname);
                out.write("\n");
            }
            removeQuery();
            // int count = MDNS.answerCount(p_hServiceQuery);  //.queryService
        }
    };

    struct freeMem : Command {
        void init() override {
            name = "freemem";
            description = "count of free memory";
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {
            out.writeInt(ESP.getFreeHeap());
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
