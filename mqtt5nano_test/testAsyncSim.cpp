// Copyright 2020 Alan Tracey Wootton
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

#include <iostream>
#include <vector>
#include <string>

#include "mqtt5nano.h"

#include <string.h> // has strcmp

using namespace std;

/** we'll pretend the bytes dribble in one at at time.
 *     as if we were getting them from async tcp
 * Let's see if we can tell when we have enoough.
 *  Do we really receive anything except publish?
 */

using namespace mqtt5nano;

slice onePacketSimpleExample();

// These hex strings were glommed from pip3 install gmqtt
// which seems to know mqtt5
// 32 is pub and qos 1
const char *XXpubbytesHex = "3268000d544553542f54494d456162636400032d080010544553542f54494d4565666768696a6b2600046b657931000476616c312600046b657932000476616c326d65737361676520617420323032302d30332d32372030313a33353a33372e34303330373920633d31";
// got this off the wire from knotfree
const char *pubbytesHex = "306600213d78484b4c487630794e5f3738344c472d736d4b363475583679324d384e6c46642808000120260006787864656267000c5b787831323334353637385d260003617477000574657374316d736723636c69656e7449642d777331333175316577745f3432";

const int pubbytesHexLen = strlen(pubbytesHex);

slice onePacketSimpleExample(mqttPacketPieces &parser, const slice availableNow);

int main()
{
    string twoPackets = pubbytesHex;
    twoPackets = twoPackets + twoPackets;
    mqttBuffer1024 testbuffer; // for convience when un hexing

    const slice pubBytesSlice = testbuffer.loadHexString(twoPackets.c_str());

    mqttPacketPieces parser;

    slice remains = onePacketSimpleExample(parser, pubBytesSlice);

    // let's pretend we have bytes dribbling in
    // porse what we can and leave the rest for later
    for (int blocksize = 1; blocksize < pubbytesHexLen; blocksize++)
    {
        slice avail = pubBytesSlice;
        avail.end = avail.start + blocksize;
        if (avail.end > pubBytesSlice.end)
        {
            avail.end = pubBytesSlice.end;
        }
        while ( ! avail.empty() )
        {
            slice remains = onePacketSimpleExample(parser, avail);
            if (remains.start == avail.start)
            { // nothing happened.
                break;
            } else {
                    // cout<< " got some? \n";
                    avail = remains;
            }
        }
    };
}

// onePacketSimpleExample attempts to parse a packet from availableNow
// returns a slice with what's left over after consuming packet, if any.
// that means the return is the same as the input if no packet was passed
slice onePacketSimpleExample(mqttPacketPieces &parser, const slice availableNow)
{
    cout << "testAsyncSim Hello World!\n";

    using namespace mqtt5nano;

    slice position(availableNow);

    if (position.size() < 2)
    {
        return availableNow;
    }
    unsigned char packetType = position.readByte();
    const int len = position.getLittleEndianVarLenInt();
    if (len == -1)
    { // not enough bytes
        return availableNow;
    }
    if (position.size() < len)
    { // there's not enough bytes in availableNow for this packet
        return availableNow;
    }
 
    parser.reset();
    bool ok = parser.parse(position, packetType, len);
    if (!ok)
    { // I don't know that we can recover from this.
        cout << "FAIL in parse \n";
        // consume everything. Hope for the best.
        position.start = position.end;
        return position;
    }

    // cout << "got one\n";

    position.start += len; //

    bool matched;
    const char *expected;
    char buffer[1024];
    matched = parser.TopicName.equals(expected = "=xHKLHv0yN_784LG-smK64uX6y2M8NlFd");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.TopicName.getCstr(buffer, 1024) << "\n";
    }
    matched = parser.Payload.equals(expected = "msg#clientId-ws131u1ewt_42");
    
    int ptype = packetType >> 4;
    if ( ptype == CtrlPublish) {
        // it was a pub
        // cout << "got pub\n";
    } 
    else {
        cout << "got a packet " << packetType << "\n";
    }

    char paybuff[1024];
    parser.Payload.getCstr(paybuff, 1024);
    if (!matched)
    {
        cout << "FAIL: Payload expected " << expected << " got " << paybuff << "\n";
    }
    matched = parser.RespTopic.equals(expected = " ");
    if (!matched)
    {
        cout << "FAIL: RespTopic expected " << expected << " got " << parser.RespTopic.getCstr(buffer, 1024) << "\n";
    }
    int i = 0;
    matched = parser.UserKeyVal[i].equals(expected = "xxdebg");
    if (!matched)
    {
        cout << "FAIL: UserKeyVal expected " << expected << " got " << parser.UserKeyVal[i].getCstr(buffer, 1024) << "\n";
    }
    matched = parser.UserKeyVal[i + 1].equals(expected = "[xx12345678]");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i + 1].getCstr(buffer, 1024) << "\n";
    }
    i = 2;
    matched = parser.UserKeyVal[i].equals(expected = "atw");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i].getCstr(buffer, 1024) << "\n";
    }
    matched = parser.UserKeyVal[i + 1].equals(expected = "test1");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i + 1].getCstr(buffer, 1024) << "\n";
    }
    i = 4;
    matched = parser.UserKeyVal[i].equals(expected = "");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i].getCstr(buffer, 1024) << "\n";
    }
    matched = parser.UserKeyVal[i + 1].equals(expected = "");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i + 1].getCstr(buffer, 1024) << "\n";
    }
    return position;
}
