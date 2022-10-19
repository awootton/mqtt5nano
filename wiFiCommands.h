
#pragma once

#include "mockStream.h"
#include "commandLine.h"
#include "eepromItem.h"
#include "timedItem.h"

#include "mockWiFi.h"
#include "mockmDNS.h"

namespace mqtt5nano
{
    extern bool connected;
    extern EepromItem ssidStash; //(32, "ssid");
    extern EepromItem passStash; // (48, "wifi pass");
    extern EepromItem hostStash; // 16
    extern int conectCountdown;
    extern bool did_mDns;

    struct reconnecter : TimedItem
    {
        class Stream *terminal;

        void execute() override
        {
            if (WiFi.status() != WL_CONNECTED)
            {
                if (conectCountdown > 0)
                {
                    // terminal->println(conectCountdown);
                    conectCountdown--;
                    return;
                }
                connected = false;
                char ssid[ssidStash.size];
                memset(ssid, 0, sizeof(ssid));

                SinkDrain ssiddrain(ssid, sizeof(ssid));
                ssidStash.read(ssiddrain);
                if (strlen(ssid) == 0)
                {
                    return;
                }
                char pass[passStash.size];
                memset(pass, 0, sizeof(pass));
                SinkDrain passdrain(pass, sizeof(pass));
                passStash.read(passdrain);
                if (strlen(pass) == 0)
                {
                    return;
                }
                WiFi.mode(WIFI_STA);
                terminal->println("WiFi connecting");
                // terminal->print(ssid);
                // terminal->print(" ");
                // terminal->println(pass);
                WiFi.begin(ssid, pass);
                conectCountdown = 60;
            }
            else
            {
                if (!connected)
                {
                    terminal->println("wifi connected");
                }
                connected = true;

                if (did_mDns == false)
                {
                    char host[32];
                    memset(host, 0, sizeof(host));
                    SinkDrain tmpdrain(host, sizeof(host));
                    hostStash.read(tmpdrain);
                    if (strlen(host) > 30)
                    {
                        // we really don't like the default.
                        slice aslice("knotfree_command_server");
                        hostStash.write(aslice);
                    }
                    if (MDNS.begin(host))
                    {
                        MDNS.setInstanceName("knot free command server");
                        MDNS.addService("http", "tcp", 80);
                        // MDNS.addServiceTxt("http","tcp");
                        did_mDns = true;
                    }
                }
            }
        }
    };
    struct WiFiHelper
    {
        reconnecter timer;

        WiFiHelper()
        {
            timer.interval = 500; // twice per sec
        }
        void loop(long now, class Stream &s)
        {
            timer.terminal = &s;
            timer.loop(now);
        }
    };

    void writeStarredPass(drain &out);

    struct ssidGet : Command
    {
        void init() override
        {
            SetName("get ssid");
            SetDescription("return WiFi ssid");
        }
        void execute(badjson::Segment *words, badjson::Segment *params, drain &out) override
        {
            out.write("ssid is: ");
            ssidStash.read(out);
        }
    };

    struct ssidSet : Command
    {
        void init() override
        {
            SetName("set ssid");
            SetDescription("set WiFi ssid");
        }
        void execute(badjson::Segment *words, badjson::Segment *params, drain &out) override
        {
            if (words == nullptr)
            {
                out.write("ERROR expected a value");
                return;
            }
            ssidStash.write(words->input);
            out.write("ok: ");
            ssidStash.read(out);
            conectCountdown = 0; // retry now
        }
    };

    struct passGet : Command
    {
        void init() override
        {
            SetName("get pass");
            SetDescription("return WiFi pass");
        }
        void execute(badjson::Segment *words, badjson::Segment *params, drain &out) override
        {
            out.write("pass is: ");
            writeStarredPass(out);
        }
    };

    struct passSet : Command
    {
        void init() override
        {
            SetName("set pass");
            SetDescription("set WiFi pss");
        }
        void execute(badjson::Segment *words, badjson::Segment *params, drain &out) override
        {
            if (words == nullptr)
            {
                out.write("ERROR expected a value");
                return;
            }
            passStash.write(words->input);
            out.write("ok: ");
            writeStarredPass(out);
            conectCountdown = 0; // retry now
        }
    };

    struct hostGet : Command
    {
        void init() override
        {
            SetName("get host");
            SetDescription("return hostname");
        }
        void execute(badjson::Segment *words, badjson::Segment *params, drain &out) override
        {
            out.write("pass is: ");
            hostStash.read(out);
        }
    };

    struct hostSet : Command
    {
        void init() override
        {
            SetName("set host");
            SetDescription("set hostname");
        }
        void execute(badjson::Segment *words, badjson::Segment *params, drain &out) override
        {
            if (words == nullptr)
            {
                out.write("ERROR expected a value");
                return;
            }
            hostStash.write(words->input);
            out.write("ok: ");
            hostStash.read(out);
            did_mDns = false; // retry now
        }
    };

    struct wifiStatus : Command
    {
        void init() override
        {
            SetName("status");
            SetDescription("return WiFi status");
        }
        void execute(badjson::Segment *words, badjson::Segment *params, drain &out) override
        {
            out.write("wifi connected: ");
            out.write(connected ? "true" : "false");
            out.write(" MDNS: ");
            out.write(did_mDns ? "true" : "false");
            out.write(" ssid: ");
            ssidStash.read(out);
            out.write(" pass: ");
            writeStarredPass(out);
            out.write(" host: ");
            hostStash.read(out);
            out.write(" ip: ");
            IPAddress addr = WiFi.localIP();
            out.writeInt(addr[0]); // eew
            out.write(".");
            out.writeInt(addr[1]);
            out.write(".");
            out.writeInt(addr[2]);
            out.write(".");
            out.writeInt(addr[3]);
        }
    };

    struct favIcon : Command
    {
        void init() override
        {
            SetName("favion.ico");
            SetDescription("return nothing");
        }
        void execute(badjson::Segment *words, badjson::Segment *params, drain &out) override
        {
            out.write("nothing");
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
