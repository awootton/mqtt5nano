

#include <iostream>
// #include <vector>
// #include <stdio.h>
#include <string>
// #include <string.h> // has strcmp

#include <stdlib.h>

/* itoa example */
#include <stdio.h>
#include <stdlib.h>

#include "mqtt5nano.h"

// #include "commandLine.h"
// #include "commonCommands.h"
// #include "streamReader.h"
// #include "wiFiCommands.h"
// #include "httpServices.h"
// #include "httpConverter.h"
// #include "badjson.h"
// #include "mqttCommands.h"
// #include "mockStream.h"
// #include "nanobase64.h"

using namespace std;
using namespace badjson;
using namespace mqtt5nano;

struct coutDrain3 : drain // for the examples output to std::cout
{
    bool writeByte(char c) override {
        std::cout << c;
        bool ok = true;
        return ok;
    };
};

int main() {

    std::cout << "hello mqtt commands tests\n";

    char rando[32];
    mqtt5nano::getRandomString(rando, sizeof(rando) - 1);
    rando[31] = 0;
    std::cout << "random is " << rando << "\n";
    coutDrain3 coutDrain;
    coutDrain.write("soe tie er wae");

    {
        char httpReq[] = "GET /get/short/name?nonc=jsjsjhhgsf HTTP/1.1\r\nHost: thing-o.local.\r\nConnection: keep-alive\r\nPragma: no-cache\r\nCache-Control: no-cache\r\n\r\n";

        slice httpReqSlice(httpReq);
        
        // char streamOutBuffer [8192];
        mqtt5nano::PackageOne one; // drag  in all the commands and timers

        DrainStream stream;
        const char cmd[] = "set ssid woot2\nset pass word4word\n";
        memcpy(stream.buff, cmd, sizeof(cmd));
        stream.source.end = sizeof(cmd);

        char serialOutBuffer[8192];
        SinkDrain sinkdrain(serialOutBuffer, sizeof(serialOutBuffer));
        stream.output = &sinkdrain;

        one.setup(stream);

        for (int ms = 100; ms < 240; ms += 3) {

            one.loop(ms, stream);

            std::cout << serialOutBuffer << " at ms:" << ms << "\n";

            if (serialOutBuffer[0] != 0) {
                sinkdrain.writeByte(0);
                std::cout << "\"" << serialOutBuffer << "\"" << ms << "\n";
                sinkdrain.reset();
                serialOutBuffer[0] = 0;
            }

            if (ms > 110) {
                one.www.client.source = &httpReqSlice;
                one.www.client.output = &coutDrain;
                // pipe in the html coutDrain
                // memcpy(serverClientInputBuffer, &httpReq[0], sizeof(httpReq));
                // serverClientInputSource.base = serverClientInputBuffer;
                // serverClientInputSource.start = 0;
                // serverClientInputSource.end = strlen(httpReq);
            }
        }
    }
    if (0) {
        // char streamOutBuffer [8192];
        mqtt5nano::PackageOne one; // drag  in all the commands and timers

        DrainStream stream;
        const char cmd[] = "set ssid woot2\nset pass word4word\n";
        memcpy(stream.buff, cmd, sizeof(cmd));
        stream.source.end = sizeof(cmd);

        char serialOutBuffer[8192];
        SinkDrain sinkdrain(serialOutBuffer, sizeof(serialOutBuffer));

        SinkDrain *cp = &sinkdrain;
        drain *dP = (drain *)cp;
        stream.output = dP;

        one.setup(stream);

        for (int ms = 100; ms < 999999; ms += 3) {
            one.loop(ms, stream);

            if (serialOutBuffer[0]) {
                std::cout << serialOutBuffer << ms << "\n";
                sinkdrain.reset();
                serialOutBuffer[0] = 0;
            }
        }
    }
    mqtt5nano::getRandomString(rando, sizeof(rando) - 1);
    std::cout << "random is " << rando << "\n";
    mqtt5nano::getRandomString(rando, sizeof(rando) - 1);
    std::cout << "random is " << rando << "\n";
    mqtt5nano::getRandomString(rando, sizeof(rando) - 1);
    std::cout << "random is " << rando << "\n";
    mqtt5nano::getRandomString(rando, sizeof(rando) - 1);
    std::cout << "random is " << rando << "\n";

    if (0) { // fixme

        Stream serial;
        // some binary for mqtt publish
        // const char *pubbytes = "3268000d544553542f54494d456162636400032d080010544553542f54494d4565666768696a6b2600046b657931000476616c312600046b657932000476616c326d65737361676520617420323032302d30332d32372030313a33353a33372e34303330373920633d31";
        char tmpbuffer[1024];
        // int amt = hex::decode((const unsigned char *)pubbytes, strlen(pubbytes), tmpbuffer, 1024);

        // client.client.source = slice(tmpbuffer, 0, amt);
        SinkDrain theDrain(tmpbuffer, 1023);

        MqttCommandClient client;

        client.client.output = &theDrain; // make drain to buffer

        client.sendSubscribe("testtopic");

        char tmpbuffer2[1024];

        slice written = theDrain.getWritten();

        int got = hex::encode((const unsigned char *)written.base, written.size(), tmpbuffer2, 1023);
        tmpbuffer2[got] = 0;

        std::cout << "\n";

        std::cout << "got " << tmpbuffer2 << "\n";

        // is this right?
        const char *need = "800f000200000974657374746f70696300";
    }
    if (0) {
        mqttPacketPieces pubgen;

        pubgen.QoS = 1;
        pubgen.packetType = CtrlPublish;

        pubgen.TopicName = "banner";
        pubgen.PacketID = 3;
        pubgen.RespTopic = "caller876543";

        pubgen.Payload = "uptime";

        // for (int i = 0; i < parser.userKeyValLen; i++) {
        //     pubgen.UserKeyVal[i] = parser.UserKeyVal[i];
        // }

        char assemblybuffer[1024]; // ?? how much 4096? new //FIXME: too big for stack
        sink assemblyArea(assemblybuffer, 1024);

        char tmpbuffer[1024];
        SinkDrain socket(tmpbuffer, 1024);

        bool ok = pubgen.outputPubOrSub(assemblyArea, &socket);
        if (!ok) {
            std::cout << "ERROR FAIL !ok \n";
        }
        char hexcmd[1024];
        int amt = hex::encode((const unsigned char *)socket.buffer.base, socket.getWritten().size(), hexcmd, 1024);
        hexcmd[amt] = 0;

        std::cout << "pub bytes hex";
        std::cout << hexcmd << "\n";

        MqttCommandClient client;

        Stream serial;
        // some binary for mqtt publish
        const char *pubbytes = hexcmd;
        char tmpbuffer3[1024];
        amt = hex::decode((const unsigned char *)pubbytes, strlen(pubbytes), tmpbuffer3, 1024);
        slice tmpSlice3(tmpbuffer3, 0, amt);
        client.client.source = &tmpSlice3;

        char cmdpoubuf[1024];
        SinkDrain cmdoutput(cmdpoubuf, 1024);
        client.client.output = &cmdoutput;

        for (int i = 0; i < 1000; i++) {

            client.loop(i, serial);

            connected = true;
            if (i == 10) {
                // throw away the connect and the subscribe
                cmdoutput.reset();
            }
        }

        char buff2[1024];

        std::cout << "got " << cmdoutput.getWritten().gethexstr(buff2, 1024) << "\n";
        std::cout << "got " << cmdoutput.getWritten().getCstr(buff2, 1024) << "\n";
    }

    if (0) {

        MqttCommandClient client;

        Stream serial;
        // some binary for mqtt publish
        const char *pubbytes = "0021206d644375784d53426c43734f436852416732316454713954787368314";
        char tmpbuffer[1024];
        int amt = hex::decode((const unsigned char *)pubbytes, strlen(pubbytes), tmpbuffer, 1024);

       // client.client.source = slice(tmpbuffer, 0, amt);

        char cmdpoubuf[1024];
        SinkDrain cmdoutput(cmdpoubuf, 1024);
        client.client.output = &cmdoutput;

        for (int i = 0; i < 1000; i++) {

            client.loop(i, serial);

            connected = true;
        }

        char buff2[1024];

        std::cout << "got " << cmdoutput.getWritten().gethexstr(buff2, 1024) << "\n";
        std::cout << "got " << cmdoutput.getWritten().getCstr(buff2, 1024) << "\n";
    }

    if (0) {

        MqttCommandClient client;

        Stream serial;
        // some binary for mqtt publish
        const char *pubbytes = "3268000d544553542f54494d456162636400032d080010544553542f54494d4565666768696a6b2600046b657931000476616c312600046b657932000476616c326d65737361676520617420323032302d30332d32372030313a33353a33372e34303330373920633d31";
        char tmpbuffer[1024];
        int amt = hex::decode((const unsigned char *)pubbytes, strlen(pubbytes), tmpbuffer, 1024);

    //    client.client.source = slice(tmpbuffer, 0, amt);

        char cmdpoubuf[1024];
        SinkDrain cmdoutput(cmdpoubuf, 1024);
        client.client.output = &cmdoutput;

        for (int i = 0; i < 1000; i++) {

            client.loop(i, serial);

            connected = true;
        }

        char buff2[1024];

        std::cout << "got " << cmdoutput.getWritten().gethexstr(buff2, 1024) << "\n";
        std::cout << "got " << cmdoutput.getWritten().getCstr(buff2, 1024) << "\n";
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
