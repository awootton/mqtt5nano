

#include <iostream>
// #include <vector>
// #include <stdio.h>
#include <string>
// #include <string.h> // has strcmp

#include <stdlib.h>

/* itoa example */
#include <stdio.h>
#include <stdlib.h>

#include "commandLine.h"

#include "commonCommands.h"

#include "streamReader.h"

#include "wiFiCommands.h"

#include "httpServices.h"

#include "httpConverter.h"

#include "badjson.h"

using namespace std;
using namespace badjson;

struct CoutDrain : drain // for the examples output to cout
{
    bool writeByte(char c) override {
        cout << c;
        bool ok = true;
        return ok;
    };
};
CoutDrain my_cout_drain;

char testbuffer[4096];

SinkDrain test_buffer_drain(testbuffer, sizeof(testbuffer));

// in real life we output to Serial or tcp or mqtt.

struct TestCommand : Command {
public:
    void init() override {
        name = "get test1";
        description = "output hello test1";
    }
    void execute(Args args, badjson::Segment *params, drain &out) override {
        out.write("hello test1");
    }
};
TestCommand testCmd;

class test1Status : TestCommand {
    void init() override {
        name = "status";
        description = "test 1 status";
    }
};
test1Status cmd1status;

int main() {
    cout << "hello command tests\n";

    {
        const char *sample = "GET /favicon.ico HTTP/1.1\r\nHost: reqbin.com\r\naccepts: *.*\r\n\r\n";
        bool ok = ParsedHttp::isWholeRequest(sample);
        cout << "got http" << ok << "\n";

        ParsedHttp parsed;
        parsed.convert(sample);
        cout << "got http" << ok << "\n";

        SinkDrain dest(testbuffer, sizeof(testbuffer));

        badjson::ToString(*parsed.command, dest);
        dest.writeByte(0);
        cout << "command " << dest.buffer.base << "\n";

        dest.reset();
        badjson::ToString(*parsed.params, dest);
        dest.writeByte(0);
        cout << "params " << dest.buffer.base << "\n";

        // delete parsed.command;
        // delete parsed.params;
    }

    {
        test_buffer_drain.buffer.reset();
        const char *test = "favicon.ico";

        ResultsTriplette res = Chop(test, strlen(test));

        CmdTestUtil::process(res.segment, nullptr, test_buffer_drain);
        // test_buffer_drain.writeByte(0); // now a cstr
        int amount = test_buffer_drain.buffer.amount();
        cout << "favicon.ico:len " << amount << "\n";

        delete res.segment; // very important
    }

    {
        const char *test = "get test1 command line";
        ResultsTriplette res = Chop("get test1 command line", strlen(test));

        Args arg(res.segment);

        int c = arg.count();

        slice a0 = arg[0];

        delete res.segment; // very important
    }

    {

        char *buffer = new char[4096];
        SinkDrain response(buffer, 4096);

        // process(parsed.command,parsed.params,response);

        // call command processing instead of this
        response.write("this would be the reply");
        int responseLen = response.buffer.start;

        cout << responseLen << "\n";
        delete[] buffer;
    }
    {
        static const int maxClients = 4;  // should be enough.?
        static const int buffSize = 1024; // should be enough.?
        WiFiClient clients[maxClients];
        char buffers[maxClients][buffSize];
        sink sinks[maxClients];
        {
            for (int i = 0; i < maxClients; i++) {
                sinks[i].base = buffers[i];
                sinks[i].start = 0;
                sinks[i].end = buffSize;
            }
        }
        for (int i = 0; i < maxClients; i++) {
            sinks[i].writeByte('G');
            sinks[i].writeByte('E');
            sinks[i].writeByte('T');
            sinks[i].writeByte(' ');
        }
        for (int i = 0; i < maxClients; i++) {
            {
                sinks[i].writeByte(0);
                cout << sinks[i].base << "\n";
                sinks[i].start--;
            }
        }
    }

    {
        // char *intStr = itoa(1234);
        std::string s = std::to_string(1234);

        SinkDrain dest(testbuffer, sizeof(testbuffer));
        dest.writeInt(123);
        dest.writeByte(0);
        cout << "command " << dest.buffer.base << "\n";
        dest.reset();
    }
    {
        const char *sample = "GET / HTTP/1.1\r\nHost: reqbin.com\r\naccepts: *.*\r\n\r\n";
        bool ok = ParsedHttp::isWholeRequest(sample);
        if (!ok) {
            cout << "isWholeRequest fail"
                 << "\n";
        }

        ParsedHttp parsed;
        parsed.convert(sample);
        cout << "got http" << ok << "\n";

        SinkDrain dest(testbuffer, sizeof(testbuffer));

        badjson::ToString(*parsed.command, dest);
        dest.writeByte(0);
        cout << "command " << dest.buffer.base << "\n";

        dest.reset();
        badjson::ToString(*parsed.params, dest);
        dest.writeByte(0);
        cout << "params " << dest.buffer.base << "\n";

        // delete parsed.command;
        // delete parsed.params;
    }

    {
        const char *sample = "GET /aaa?kk HTTP/1.1\r\nHost: reqbin.com\r\naccepts: *.*\r\n\r\n";
        bool ok = ParsedHttp::isWholeRequest(sample);
        cout << "got http" << ok << "\n";

        ParsedHttp parsed;
        parsed.convert(sample);
        cout << "got http" << ok << "\n";

        SinkDrain dest(testbuffer, sizeof(testbuffer));

        badjson::ToString(*parsed.command, dest);
        dest.writeByte(0);
        cout << "command " << dest.buffer.base << "\n";

        dest.reset();
        badjson::ToString(*parsed.params, dest);
        dest.writeByte(0);
        cout << "params " << dest.buffer.base << "\n";

        // delete parsed.command;
        // delete parsed.params;
    }

    {
        const char *sample = "GET /echo/me HTTP/1.1\r\nHost: reqbin.com\r\naccepts: *.*\r\n\r\n";
        bool ok = ParsedHttp::isWholeRequest(sample);
        cout << "got http" << ok << "\n";

        ParsedHttp parsed;
        parsed.convert(sample);
        cout << "got http" << ok << "\n";

        SinkDrain dest(testbuffer, sizeof(testbuffer));

        badjson::ToString(*parsed.command, dest);
        dest.writeByte(0);
        cout << "command " << dest.buffer.base << "\n";

        dest.reset();
        badjson::ToString(*parsed.params, dest);
        dest.writeByte(0);
        cout << "params " << dest.buffer.base << "\n";

        // delete parsed.command;
        //  delete parsed.params;
    }

    {
        const char *sample = "GET /echo/me?key1=val1&k2=v2 HTTP/1.1\r\nHost: reqbin.com\r\naccepts: *.*\r\n\r\n";
        bool ok = ParsedHttp::isWholeRequest(sample);
        cout << "got http" << ok << "\n";

        ParsedHttp parsed;
        parsed.convert(sample);
        cout << "got http" << ok << "\n";

        SinkDrain dest(testbuffer, sizeof(testbuffer));

        badjson::ToString(*parsed.command, dest);
        dest.writeByte(0);
        cout << "command " << dest.buffer.base << "\n";

        dest.reset();
        badjson::ToString(*parsed.params, dest);
        dest.writeByte(0);
        cout << "params " << dest.buffer.base << "\n";

        // delete parsed.command;
        // delete parsed.params;
    }
    {
        test_buffer_drain.buffer.reset();
        const char *test = "abc def";

        ResultsTriplette res = Chop(test, strlen(test));

        CmdTestUtil::process(res.segment, nullptr, test_buffer_drain);
        test_buffer_drain.writeByte(0); // now a cstr
        cout << "" << test_buffer_drain.buffer.base << "\n";

        delete res.segment; // very important
    }

    {
        const char *test = "get test1 command line";
        ResultsTriplette res = Chop("get test1 command line", strlen(test));

        CmdTestUtil::process(res.segment, nullptr, my_cout_drain);
        cout << "\n";

        test_buffer_drain.buffer.reset();
        CmdTestUtil::process(res.segment, nullptr, test_buffer_drain);
        test_buffer_drain.writeByte(0); // now a cstr
        cout << test_buffer_drain.buffer.base << "\n";
        if (strcmp(test_buffer_drain.buffer.base, "hello test1\n")) {
            cout << "FAIL didn't get expected\n";
        }
        delete res.segment; // very important
    }
    {
        test_buffer_drain.buffer.reset();
        const char *test = "get dummy1";

        ResultsTriplette res = Chop(test, strlen(test));

        CmdTestUtil::process(res.segment, nullptr, test_buffer_drain);
        test_buffer_drain.writeByte(0); // now a cstr
        cout << "get dummy1: " << test_buffer_drain.buffer.base << "\n";

        delete res.segment; // very important
    }
    {
        test_buffer_drain.buffer.reset();
        const char *test = "set dummy1 newDummyVal";

        ResultsTriplette res = Chop(test, strlen(test));

        CmdTestUtil::process(res.segment, nullptr, test_buffer_drain);
        test_buffer_drain.writeByte(0); // now a cstr
        cout << "set dummy1: " << test_buffer_drain.buffer.base << "\n";

        delete res.segment; // very important
    }
    {
        test_buffer_drain.buffer.reset();
        const char *test = "get dummy1";

        ResultsTriplette res = Chop(test, strlen(test));

        CmdTestUtil::process(res.segment, nullptr, test_buffer_drain);
        test_buffer_drain.writeByte(0); // now a cstr
        cout << "get dummy1: " << test_buffer_drain.buffer.base << "\n";

        delete res.segment; // very important
    }
    {
        test_buffer_drain.buffer.reset();
        const char *test = "status";

        ResultsTriplette res = Chop(test, strlen(test));

        CmdTestUtil::process(res.segment, nullptr, test_buffer_drain);
        test_buffer_drain.writeByte(0); // now a cstr
        cout << "status: " << test_buffer_drain.buffer.base << "\n";

        delete res.segment; // very important
    }
    {
        test_buffer_drain.buffer.reset();
        const char *test = "help";

        ResultsTriplette res = Chop(test, strlen(test));

        CmdTestUtil::process(res.segment, nullptr, test_buffer_drain);
        test_buffer_drain.writeByte(0); // now a cstr
        cout << "help: " << test_buffer_drain.buffer.base << "\n";

        delete res.segment; // very important
    }
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
