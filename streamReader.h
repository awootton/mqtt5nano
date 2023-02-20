
#pragma once

#include "nanoCommon.h"
#include "badjson.h"
#include "commandLine.h"
#include "mockStream.h"
#include "slices.h"

namespace mqtt5nano {
    /**  This is just another implementation of
     * the Usual Serial.read() that buffers the bytes up
     * until there's a whole command.
     * And then it parses the line into words and then passes it to the commands.
     */

    struct StreamDrain : drain {
        Stream *sP = nullptr;
        StreamDrain(Stream &streams)
            : sP(&streams) {
        }
        StreamDrain() {}

        bool writeByte(char c) override {
            int amt = sP->print(c);
            bool ok = amt == 1;
            return ok;
        };
    };

    struct streamReader {
        sink ourBuffer;
        Stream *sP;
        StreamDrain adrain;

        static const int readBufferSize = 1024;
        char readbuffer[readBufferSize];

        streamReader() {
            setup();
        }

        void setup() {
            ourBuffer.base = readbuffer;
            ourBuffer.start = 0;
            ourBuffer.end = readBufferSize;
            readbuffer[readBufferSize - 1] = 0;
        }

        // streamReader(char *buffer, int bufflen)
        // {
        //     ourBuffer.base = readbuffer;
        //     ourBuffer.start = 0;
        //     ourBuffer.end = readBufferSize;
        //     buffer[readBufferSize - 1] = 0;
        // }
        virtual slice haveEnough() {
            if (ourBuffer.start) {
                char c = ourBuffer.base[ourBuffer.start - 1];
                if (c == '\n' || c == '\r') {
                    slice newSlice(ourBuffer.base, 0, ourBuffer.start - 1); // don't pass the \n
                    ourBuffer.start = 0;                                    // reset the buffer
                    return newSlice;
                }
            }
            return slice();
        }
        virtual void execute(slice cmd) // override me ? FIXME: use the layers pattern
        {
            // sP->print("received command: ");
            // sP->println(cmd.base);
            badjson::ResultsTriplette chopped = badjson::Chop(cmd.base + cmd.start, cmd.size());
            if (chopped.error == nullptr) { // process command line
                Command::process(chopped.segment, nullptr, adrain, CommandSource::SerialPort);
            } else {
                adrain.write(chopped.error);
                adrain.write("\n");
            }
            delete chopped.segment;
        }
        void loop(long now, class Stream &s) {
            sP = &s;
            adrain.sP = &s;
            while (s.available()) {
                // fixme call pipeline aka layers.
                char c = s.read();
                ourBuffer.writeByte(c);
                slice cmd = haveEnough();
                if (!cmd.empty()) {
                    moreScrambled(now);
                    execute(cmd); 
                }
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
