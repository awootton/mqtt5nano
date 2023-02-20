
#include "commandLine.h"

#include <string.h> // has strcmp

#include "badjson.h"

namespace mqtt5nano {

    Command *head = 0;

    Command *
    getHead() {
        return head;
    }

    Command::Command() {
        next = head;
        head = this;
    }

    void
    Command::execute(Args args, badjson::Segment *params, drain &out) {
        out.write("override me");
    }

    // this returns true if the seg1 segment list matches the beginning of the
    // incomingCommandLine
    bool
    matchSegments(badjson::Segment &seg1, badjson::Segment &incomingCommandLine) {
        badjson::Segment *s1 = &seg1;
        badjson::Segment *s2 = &incomingCommandLine;
        while (s1 != nullptr && s2 != nullptr) {
            if (s1->input.equals(s2->input) == false) {
                return false;
            }
            s1 = s1->Next();
            s2 = s2->Next();
        }
        if (s2 == nullptr) { // we used the whole command line
            // then we should have used the whole command also
            // if we didn't then they don't really match.
            if (s1 == nullptr) {
                // ok
            } else {
                return false;
            }
        }
        return true;
    }

    class HelpCommand : Command {

    public:
        void
        init() override {
            name = "help";
            description = "lists all commands. 🔓 means no encryption required";
        }
        void
        execute(Args args, badjson::Segment *params, drain &out) override {
            Command *cmd = getHead();
            int count = 0;
            while (cmd) {
                count++;
                cmd = cmd->next;
            }
            Command *cmds[count];
            cmd = getHead();
            int i = 0;
            while (cmd) {
                cmds[i] = cmd;
                i++;
                cmd = cmd->next;
            }
            // sort the list in cmd
            for (int i = 0; i < count; i++) {
                for (int j = i + 1; j < count; j++) {
                    if (strcmp(cmds[i]->name, cmds[j]->name) > 0) {
                        Command *tmp = cmds[i];
                        cmds[i] = cmds[j];
                        cmds[j] = tmp;
                    }
                }
            }

            for (int i = 0; i < count; i++) {
                cmd = cmds[i];
                // out.write(cmd->name);
                // this one quotes the words and I don't like that
                // ToString(*cmd->parsed, out);
                out.write("[");
                badjson::Segment *s = cmd->parsed;
                while (s != nullptr) {
                    out.write(s->input);
                    s = s->next;
                    if (s != nullptr) {
                        out.write(" ");
                    }
                }
                out.write("]");
                if (cmd->argumentCount != 0) {
                    out.write(" +");
                    out.writeInt(cmd->argumentCount);
                }
                out.write(" ");
                const char *desc = cmd->description;
                if (desc == nullptr) {
                    desc = cmd->name;
                }
                out.write(desc);
                if (cmd->next) {
                    out.write("\n");
                } else {
                    out.write("\n"); // TODO: get it right.
                }
                cmd = cmd->next;
            }
        }

        void test() { // for the linkedin article
            Command *cmd = getHead();
            int count = 0;
            while (cmd) {
                count++;
                cmd = cmd->next;
            }
            Command *cmds[count];
            cmd = getHead();
            int i = 0;
            while (cmd) {
                cmds[i] = cmd;
                i++;
                cmd = cmd->next;
            }

            // sort the list in cmd
            for (int i = 0; i < count; i++) {
                for (int j = i + 1; j < count; j++) {
                    if (strcmp(cmds[i]->name, cmds[j]->name) > 0) {
                        Command *tmp = cmds[i];
                        cmds[i] = cmds[j];
                        cmds[j] = tmp;
                    }
                }
            }
        }
    };

    HelpCommand helpcmd;

    int commandsServed = 0;

    // precess walks through the list of command looking for a match
    // Note that we execute all the matches even if more than one.
    void
    Command::process(badjson::Segment *incomingCommandLine,
                     badjson::Segment *params, drain &out, CommandSource source) {
        // out.write("# Command::process");
        Command *command = head;
        while (command != nullptr) {
            if (command->name[0] == 0) { // lazy init
                command->init();
                command->parseTheName(); // allocs memory.
            }
            command = command->next;
        }
        command = head;
        bool foundOne = false;
        while (command != nullptr) {
            if (matchSegments(*command->parsed, *incomingCommandLine)) { // we don't want to pass the command to
                // the execute, just the args
                // I could rig matchSegments fins args but that's a hack.
                badjson::Segment *args = incomingCommandLine;
                badjson::Segment *tmp = command->parsed;
                while (tmp) { // walk the list
                    tmp = tmp->next;
                    args = args->next;
                    if (args == nullptr) {
                        break;
                    }
                }
                foundOne = true;
                Args cargs(args);
                cargs.source = source;
                command->execute(cargs, params, out);
                commandsServed++;
                // do we have to? this fucks up favicon:
                out.write("\n");
            }
            command = command->next;
        }
        if (foundOne == false) {
            out.write("Command not found:\n");
            badjson::ToString(*incomingCommandLine, out);
            out.write("\n");
            out.write("See help for available commands:\n");
            foundOne = true;
            Args cargs(incomingCommandLine);
            cargs.source = source;
            helpcmd.execute(cargs, nullptr, out);
        }
    }

    void
    Command::parseTheName() // since the name can be several words, chop it
                            // up.
    {
        if (name == nullptr) {
            return; // needs error
        }
        if (strlen(name) == 0) {
            return; // needs error
        }
        if (parsed != nullptr) {
            return; // we already did it. dry.
        }
        badjson::ResultsTriplette res = badjson::Chop(name, strlen(name));
        parsed = res.segment;
    }
} // namespace mqtt5nano

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
