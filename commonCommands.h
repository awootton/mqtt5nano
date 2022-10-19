

#pragma once

#include "commandLine.h"
#include "eepromItem.h"

namespace mqtt5nano
{
    struct getUptime : Command
    {
        void init() override
        {
            SetName("uptime");
            SetDescription("return time since last reboot");
        }
        void execute(badjson::Segment *words, badjson::Segment *params, drain &out) override
        {
            long now = latestNowMillis;
            now = now / 1000;
            int secords = now % 60;
            now = now / 60;
            int minutes = now % 60;
            now = now / 60;
            int hours = now;
            out.writeInt(hours);
            out.write(" hours ");
            out.writeInt(minutes);
            out.writeByte(':');
            out.writeInt(minutes);
        }
    };

    struct getServed : Command
    {
        void init() override
        {
            SetName("served");
            SetDescription("count of requests served since reboot");
        }
        void execute(badjson::Segment *words, badjson::Segment *params, drain &out) override
        {
            out.writeInt(commandsServed);
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
