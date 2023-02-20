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
            SetInterval(10 * 1000); // every 10 sec.
        };
        void execute() {
            if (connected) {
                mdnsUpdate(); // MDNS.update();
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
        sink websink;
        bool clientconnected;

        WebServer() : server(80) {

            websink.base = sinkbuffer;
            websink.start = 0;
            websink.end = buffSize;
        }
        void serviceClients(class Stream &s) {
            if (client.connected()) {
                if (!clientconnected) {
                    // s.println("# newly connected ");
                    // s.println();
                    clientconnected = true;
                }
                int charMax = 64;
                while (charMax && client.available() > 0) {
                    charMax--;
                    if ((websink.start % 16) == 0) {
                        // s.print(".");
                    }
                    char c = client.read();
                    websink.writeByte(c);
                    if (ParsedHttp::isWholeRequest(websink)) {
                        // got one !!
                        {
                            s.println("# got http ");
                            websink.base[websink.start] = 0; // null terminate it
                            s.println(websink.base);
                        }
                        // s.println(0);
                        // I'm gonna need a buffer
                        // char *cmdreplybuffer = new char[2048];
                        // SinkDrain response(cmdreplybuffer, 2048);

                        CommandPipeline pipeline;
                        pipeline.fromHttp = true;


                        StreamDrain dest(client);
                      
                        pipeline.handlePayload(websink.getWritten(), dest);

                        // ParsedHttp httpprops;
                        // bool ok = httpprops.convert(websink);
                        // if (ok) {
                        //     // s.println("# parsed ok");
                        //     // StreamDrain dr(s);
                        //     // parsed.command->GetQuoted(dr); // print the command
                        //     // s.println();
                        // } else {
                        //     s.println("# parsed FAIL");
                        //     s.println(websink.getWritten().getCstr(cmdreplybuffer,2048));
                        // }
                        // // pass to execute
                        // Command::process(httpprops.command, httpprops.params, response);

                        // int responseLen = response.buffer.start;
                        // s.print("content len ");
                        // s.println(responseLen);

                        // bool isPng = httpprops.command->input.equals("favicon.ico");

                        // if (!isPng) {
                        //     // response.writeByte((char)0);
                        // }
                        // s.print("content");
                        // s.println(response.buffer.base);

                        // TODO: out-line this
                        // if (true)
                        // {
                        //     client.print(F("HTTP/1.1 200 OK\r\n"));
                        //     client.print(F("Content-Length: "));
                        //     client.print(responseLen);
                        //     client.print(F("\r\n"));
                        //     // we have to return the params as headers
                        //     badjson::Segment *pP = httpprops.params;
                        //     char tmp[64];
                        //     while (pP != nullptr) {
                        //         client.write(pP->input.getCstr(tmp, sizeof(tmp)));
                        //         client.write(": ");
                        //         pP = pP->nexts;
                        //         if (pP != nullptr) {
                        //             client.write(pP->input.getCstr(tmp, sizeof(tmp)));
                        //             pP = pP->nexts;
                        //         }
                        //         client.print(F("\r\n"));
                        //     }
                        //     if (isPng) { // hack alert
                        //         client.print(F("Content-Type: image/png\r\n"));
                        //     } else {
                        //         client.print(F("Content-Type: text/plain\r\n"));
                        //     }
                        //     client.print(F("Access-Control-Allow-Origin: *\r\n"));
                        //     client.print(F("Access-control-expose-headers: nonc\r\n"));
                        //     client.print(F("Connection: Closed\r\n"));
                        //     client.print(F("\r\n"));
                        // }

                        // wtf client.write((const char*)cmdreplybuffer,(size_t)responseLen);
                        // for (int i = 0; i < responseLen; i++) {
                        //     client.write( response.buffer.base[i] );
                        // }

                        client.flush();
                        client.stop();
                        // delete[] cmdreplybuffer;
                        // delete parsed.command; happens auto at }
                        // delete parsed.params; happens auto at }

                        websink.start = 0; // reset
                    }
                }
            } else {
                if (clientconnected) {
                    // s.println("# UN connected ");
                    // s.println(i);
                    clientconnected = false;
                }
            }
        }

        void loop(long now, class Stream &s) {
            if (connected) {
                // open a listener socket
                if (!didServerInit) {
                    server.begin();
                    didServerInit = true;
                    // s.println("# Http server begin");
                    // MDNS.addService("http", "tcp", 80);
                }
                bool anyConnected = client.connected();
                if (!anyConnected) {
                    client = server.available(); // does it new these?
                }
                serviceClients(s);
            } else {
                // s.println("server gave us an unconnected Client !!! ");
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
