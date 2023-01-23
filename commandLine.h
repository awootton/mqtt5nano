
#pragma once

#include "badjson.h"

/** Command is the base class for all the command handlers.
 * All you have to do is declare one and it automatically gets
 * linked in with the rest of them.
 */
namespace mqtt5nano {
    
    extern int commandsServed;
    extern long latestNowMillis;

    struct permissionType {
        bool notAllowed;
        bool requiresEncryption;
    };

    struct PermissionGroup {
        permissionType Serial;     // permissions to receive commands from the Serial port.
        permissionType Local;      // permissions to receive commands from the local network.
        permissionType Everywhere; // // permissions to receive commands from the internet.
    };

    struct Args {
    private:
        badjson::Segment *in;

    public:
        Args(badjson::Segment *incomingCommandLine) {
            in = incomingCommandLine;
        }
        int count() {
            int c = 0;
            badjson::Segment *s = in;
            while (s != nullptr) {
                c++;
                s = s->nexts;
            }
            return c;
        }
        slice operator[](int i) const {
            int c = 0;
            badjson::Segment *s = in;
            while (s != nullptr && c++ < i) {
                c++;
                s = s->nexts;
            }
            if (s != nullptr) {
                return s->input;
            }
            return slice("");
        }
    };

    class Command // the virtual base class. Aka the interface.
    {
        friend class HelpCommand;
        friend struct streamReader;
        friend struct WebServer;
        friend struct MqttCommandClient;
        friend class CmdTestUtil;

    private:
        Command *next;
        badjson::Segment *parsed = nullptr;

    public:
        const char *name = "";
        const char *description = "";
        PermissionGroup permissions;
        int argumentCount = 0;

        Command();
        virtual void execute(Args args, badjson::Segment *params, drain &out);

    private:
        virtual void init(){};
        void parseTheName(); // since the name can be several words, chop it up
        static void process(badjson::Segment *incomingCommandLine, badjson::Segment *params, drain &out);
    };

    Command *getHead();

    class CmdTestUtil {
    public:
        static void process(badjson::Segment *incomingCommandLine, badjson::Segment *params, drain &out) {
            Command::process(incomingCommandLine, params, out);
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
