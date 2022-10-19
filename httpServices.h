#pragma once

#include "wiFiCommands.h"
// #include "commandLine.h"
// #include "EepromItem.h"
// #include "timedItem.h"

#include "mockWiFi.h"
// #include "knotmDNS.h"

#include "httpConverter.h"
#include "streamReader.h"

namespace mqtt5nano
{

    /** WebServer is not a typical web server. Right now it only accepts GET
     * and it simply removes the '/' from the path and then passes that to the usual
     * command handling chain here.
     */
    struct WebServer // TODO: please clean up.
    {
        WiFiServer server; //(80);
        bool didServerInit = false;

        // can we afford 4k? Multiple clients didn't work.
        // static const int maxClients = 4;  // should be enough.?
        static const int buffSize = 1024; // should be enough.?
        WiFiClient client;
        // struct bb
        // {
        //     char buf[buffSize];
        // };
        char sinkbuffer[buffSize]; // s[maxClients]; // [maxClients][buffSize];
        sink websink;
        bool clientconnected; //[maxClients];

        WebServer() : server(80)
        {

            websink.base = sinkbuffer;
            websink.start = 0;
            websink.end = buffSize;
            //   clientconnected[i] = false;
        }
        void serviceClients(class Stream &s)
        {
            // service the clients.
            // for (int i = 0; i < maxClients; i++)
            //{
            if (client.connected())
            {
                if (!clientconnected)
                {
                    s.println("newly connected ");
                    // s.println();
                    clientconnected = true;
                }
                int charMax = 32;
                // WiFiClient &client = clients[i];
                while (charMax && client.available() > 0)
                {
                    charMax--;
                    if ((websink.start % 16) == 0)
                    {
                        s.print(".");
                    }
                    char c = client.read();
                    websink.writeByte(c);
                    if (ParsedHttp::isWholeRequest(websink))
                    {
                        // got one !!
                        s.println("got http ");
                        // s.println(0);
                        // I'm gonna need a buffer
                        char *cmdreplybuffer = new char[2048];
                        SinkDrain response(cmdreplybuffer, 2048);

                        ParsedHttp parsed;
                        bool ok = parsed.convert(websink);
                        if (ok)
                        {
                            s.println("parsed ok");
                            // s.print(i);
                            StreamDrain dr(s);
                            parsed.command->GetQuoted(dr);
                            s.println();
                        }
                        else
                        {
                            s.println("parsed FAIL");
                            // s.println(i);
                        }
                        // pass to execute
                        process(parsed.command, parsed.params, response);

                        // call command processing instead of this
                        // response.write("this would be the reply");
                        int responseLen = response.buffer.start;
                        response.writeByte((char)0);

                        // s.print("content len");
                        // s.println(responseLen);

                        // s.print("content");
                        // s.println(response.buffer.base);

                        if (true)// can we skip all this?? Seems to work sometimes. Kinda mean though. Good to not 'new' buffer though.
                        {
                            client.write("HTTP/1.1 200 OK\r\n"); 
                            client.write("Content-Length: ");
                            client.print(responseLen);
                            client.write("\r\n");
                            client.write("Content-Type: text/plain\r\n");
                            client.write("Connection: Closed\r\n");
                            client.write("\r\n");
                        }

                        // client.setNoDelay(true);
                        client.write(cmdreplybuffer);

                        client.flush();
                        client.stop();
                        delete[] cmdreplybuffer;
                        // delete parsed.command;
                        // delete parsed.params;

                        websink.start = 0; // reset
                    }
                }
            }
            else
            {
                if (clientconnected)
                {
                    s.println("UN connected ");
                    // s.println(i);
                    clientconnected = false;
                }
            }
            //}
        }

        void loop(long now, class Stream &s)
        {
            if (connected)
            {
                // open a listener socket
                if (!didServerInit)
                {
                    server.begin();
                    didServerInit = true;
                    s.println("server begin");
                }
                bool anyConnected = client.connected();
                if (!anyConnected)
                {
                    client = server.available(); // does it new these?
                    // if (client.connected())
                    // {
                    //     s.println("new client.connected");
                    //     // we have a new one
                    //     bool found = false;
                    //     for (int i = 0; i < maxClients; i++)
                    //     {
                    //         if (clients[i] == client)
                    //         {
                    //             s.print("found ");
                    //             s.println(i);
                    //             found = true;
                    //             break;
                    //         }
                    //     }
                    //     if (!found)
                    //     {
                    //         for (int i = 0; i < maxClients; i++)
                    //         {
                    //             if (!(clients[i].connected()))
                    //             {
                    //                 clients[i] = client;
                    //                 s.print("added to slot ");
                    //                 s.println(i);
                    //                 break;
                    //             }
                    //         }
                    //     }
                    // }
                }
                serviceClients(s);
            }
            else
            {
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
