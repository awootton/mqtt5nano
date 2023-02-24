#pragma once

#include "nanoCommon.h"

#include "wiFiCommands.h"

#include "mockWiFi.h"

#include "httpConverter.h"
#include "streamReader.h"

#include "layers.h"
#include "streamReader.h"

// this is for local mode.

namespace mqtt5nano {

    struct refreshMdnsTimer : TimedItem {
        void init() {
            SetInterval(1 * 1000); // every 1 sec.
        };
        void execute() {
            if (connected) {
                mdnsUpdate(); // MDNserialDestination.update();
            }
        }
    };

    /** WebServer is not a typical web server. Right now it only accepts GET
     * and it simply removes the '/' from the path and then passes that to the usual
     * command handling chain here.
     */
    struct WebServer // TODO: please clean up.
    {
        WiFiServer server; //(80);
        bool didServerInit = false;

        refreshMdnsTimer mdnsTimer;

        // can we afford 4k? Multiple clients didn't work.
        // static const int maxClients = 4;  // should be enough.?
        static const int buffSize = 1024; // should be enough.?
        WiFiClient client;
        char sinkbuffer[buffSize]; // s[maxClients]; // [maxClients][buffSize];
        ByteCollector websink;
        bool clientconnected;

        WebServer() : server(80) {

            websink.base = sinkbuffer;
            websink.start = 0;
            websink.end = buffSize;
        }
        void serviceClient() {
            if (client.connected()) {
                if (!clientconnected) {
                    serialDestination.println("\n\n\n# newly connected ");
                    // serialDestination.println();
                    clientconnected = true;
                }
                int charMax = websink.remaining();
                while (charMax && client.available() > 0) {
                    charMax--;
                    if ((websink.start % 16) == 0) {
                        // serialDestination.print(".");
                    }
                    char c = client.read();
                    websink.writeByte(c);
                    if (ParsedHttp::isWholeRequest(websink)) {
                        // got one !!

                        // serialDestination.print("# got http ", slice(websink), "\n");

                        CommandPipeline pipeline;
                        pipeline.fromHttp = true;

                        StreamDestination dest(client);

                        pipeline.handlePayload(websink.getWritten(), dest);

                        client.flush();
                        client.stop();

                        websink.start = 0; // reset
                    }
                }
            } else {
                if (clientconnected) {
                    // serialDestination.println("# UN connected ");
                    // serialDestination.println(i);
                    clientconnected = false;
                }
            }
        }

        int lastTime = 0;
        void loop(long now, class Stream &s) {
            if (connected) {
                // open a listener socket
                // let's try to do this less often.
                if (now - lastTime > 1000) {
                    lastTime = now;
                    if (!didServerInit) {
                        server.begin();
                        didServerInit = true;
                        serialDestination.println("# Http server begin");
                        // MDNserialDestination.addService("http", "tcp", 80);
                    }
                    bool anyConnected = client.connected();
                    if (!anyConnected) {
                        client = server.available(); // does it new these? or fill them in or what?
                    }
                    serviceClient();
                }     
            } else {
                // serialDestination.println("server gave us an unconnected Client !!! ");
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
// GNU General Public License for more detailserialDestination.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
