
#pragma once

#include "badjson.h"

/** Command is the base class for all the command handlers.
 * All you have to do is declare one and it automatically gets
 * linked in with the rest of them.
 */  
namespace mqtt5nano
{
    extern int commandsServed; 
    extern long latestNowMillis;

    struct Command // the virtual base class. Aka the interface.
    {
        Command *next;
        const char *name = "";
        const char *description = "";
        badjson::Segment *parsed = nullptr;

        Command();
        Command(const char *name, const char *decription);
        void SetName(const char *name)
        {
            this->name = name;
            parseTheName();
        }
        void SetDescription(const char *desc)
        {
            this->description = desc;
        }
        // params are key val key val etc.
        virtual void execute(badjson::Segment *words,badjson::Segment *params, drain &out);
        virtual void init(){};
        void parseTheName();// since the name can be several words, chop it up
    };

    // serial and now?
    void process(badjson::Segment *words, badjson::Segment *params,drain &out);

    Command * getHead();
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
