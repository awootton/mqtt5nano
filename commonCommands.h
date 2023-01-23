

#pragma once

#include "commandLine.h"
#include "eepromItem.h"

#include "nanoCrypto.h"

namespace mqtt5nano {

    
    struct getUptime : Command {

        int starttime = 0;
        void init() override {
            name = "uptime";
            description = "time since last reboot.";
            starttime = latestNowMillis;
        }
        void execute(Args args, badjson::Segment *params, drain &out) override {
            long delta = latestNowMillis - starttime;
            delta = delta / 1000;
            int seconds = delta % 60;
            delta = delta / 60;
            int minutes = delta % 60;
            delta = delta / 60;
            int hours = delta;
            out.writeInt(hours);
            out.write(" hours ");
            out.writeInt(minutes);
            out.writeByte(':');
            out.writeInt(seconds);
        }
    };

    struct getServed : Command {
        void init() override {
            name = "served";
            description = "count of requests served since reboot";
        }
        void execute(Args args, badjson::Segment *params, drain &out) override {
            out.writeInt(commandsServed);
        }
    };

    struct getVersion : Command {
        void init() override {
            name = "version";
            description = "mqtt5nano version";
        }
        void execute(Args args, badjson::Segment *params, drain &out) override {
            out.write("v0.1.0");
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
