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

/** This main() runs the tests that we have. 
 * The outout should be free of the word "FAIL".
 * We should assume that any mqtt functionality that is not in these tests is broken. 
 */

void testParsePublish();      // below
void testGenerateSubscribe(); // annoying
void testGeneratePublish();   // need go now.
void testGenerateConnect();   // declared below
void testParseMisc();         // declared below

void testParsePublish2();      // below

void testendians();


int main()
{

    cout << "Hello World!\n";

    // we need to parse a pub and generate a pub.
    // we need to be able to generate a sub and generate a connect.
    // we need to not barf, ever ever ever, when receiving garbage.
    using namespace mqtt5nano;

    testParsePublish2();
   
    testParseMisc();

    testGenerateConnect();

    testGeneratePublish();

    testGenerateSubscribe();

    testParsePublish();
}

using namespace mqtt5nano;

bool parseHelper(mqttPacketPieces &parser, mqttBuffer1024 &parserBuffer, slice net)
{
    bool fail = false;

    while (!net.empty())
    {
        unsigned char packetType = net.readByte();
        int len = net.getLittleEndianVarLenInt();

        if (len > 1024 - 4)
        {
            fail = true;
            return fail;
        }
        if (len == 0) // is legit
        {
            fail = false;
            return fail;
        }

        // now we can read the rest
        //  mqttBuffer parserBuffer;
        slice body = net ;//parserBuffer.loadFromFount(*net, len);
        body.end = body.start + len;
        net.start += len;

        fail = parser.parse(body, packetType, len);
        if (fail)
        {
            cout << "FAIL in parse test \n";
            return fail;
        }
    }
    return fail;
}

void testParseMisc()
{
    using namespace mqtt5nano;

    mqttBuffer1024 testbuffer;   // to supply the fount
    mqttBuffer1024 parserBuffer; // for use by the parser.

    mqttPacketPieces parser; // the parser.
    slice net;

    const char *pubbytes = "200600000322000a";// this is NOT a pub

    net = testbuffer.loadHexString(pubbytes);

    bool failed = parseHelper(parser, parserBuffer, net);

    if (failed)
    {
        cout << "FAIL of some kind \n";
    }

    pubbytes = "900400010000";// not a pub

    net = testbuffer.loadHexString(pubbytes);

    failed = parseHelper(parser, parserBuffer, net);

    if (failed)
    {
        cout << "FAIL of some kind \n";
    }

    pubbytes = "40020004";

    net = testbuffer.loadHexString(pubbytes);

    failed = parseHelper(parser, parserBuffer, net);

    if (failed)
    {
        cout << "FAIL of some kind \n";
    }

    // now, all in a row.
    pubbytes = "200600000322000a90040001000040020004";

    net = testbuffer.loadHexString(pubbytes);

    failed = parseHelper(parser, parserBuffer, net);

    if (failed)
    {
        cout << "FAIL of some kind \n";
    }
    // what is in it?



    cout << "testParseMisc finished\n" ;
}

void testGenerateConnect()
{
    using namespace mqtt5nano;

    slice s;
    mqttPacketPieces congen;
    congen.reset();

    mqttBuffer1024 buffer;
    sink buffersink = buffer.getSink();

    mqttBuffer1024 result;
    SinkDrain netDrain;
    netDrain.buffer = result.getSink();

    slice clientID = slice("client-id-uwjnbnegfgtwfqk");
    slice userName = slice("abc");
    slice passWord = slice("123");

    bool ok = congen.outputConnect(buffersink, &netDrain, clientID, userName, passWord);

    slice rsl = netDrain.buffer.getWritten();
    char tmpbuffer[1024];

    // these bytes came from gmqtt and were tested on mosquitto.
    // should be:
    const char *need = "103000044d51545405c2003c000019636c69656e742d69642d75776a6e626e65676667747766716b00036162630003313233";
    const char *got = rsl.gethexstr(tmpbuffer,1024);
    if (strcmp(got, need) != 0)
    {
        cout << "testGenerateConnect FAIL: expected --" << need << "-- got --" << got << "--\n";
    }
}

void testGeneratePublish()
{
    using namespace mqtt5nano;

    slice s;
    mqttPacketPieces pubgen;
    pubgen.reset();

    pubgen.QoS = 1;
    pubgen.packetType = CtrlPublish;

    pubgen.TopicName = slice("TEST/TIMEabcd");
    pubgen.PacketID = 3;
    pubgen.RespTopic = slice("TEST/TIMEefghijk");

    pubgen.UserKeyVal[0] = slice("key1");
    pubgen.UserKeyVal[1] = slice("val1");
    pubgen.UserKeyVal[2] = slice("key2");
    pubgen.UserKeyVal[3] = slice("val2");
    pubgen.Payload = slice("message at 2020-03-27 01:35:37.403079 c=1");

    mqttBuffer1024 buffer;
    sink buffersink = buffer.getSink();

    mqttBuffer1024 result;
    SinkDrain netDrain;
    netDrain.buffer = result.getSink();

    bool ok = pubgen.outputPubOrSub(buffersink, &netDrain);

    slice rsl = netDrain.buffer.getWritten();
    char tmpbuffer[1024];

    // should be:
    // same as pubbytes in testParsePublish
    const char *need = "3268000d544553542f54494d456162636400032d080010544553542f54494d4565666768696a6b2600046b657931000476616c312600046b657932000476616c326d65737361676520617420323032302d30332d32372030313a33353a33372e34303330373920633d31";
    const char *got = rsl.gethexstr(tmpbuffer,1024);
    if (strcmp(got, need) != 0)
    {
        cout << "testGeneratePublish FAIL: expected " << need << " got " << got << "\n";
    }
}

void testGenerateSubscribe()
{

    using namespace mqtt5nano;

    slice s;
    mqttPacketPieces subscribe;
    subscribe.reset();

    subscribe.PacketID = 2;
    // add props?
    // payload is the topic
    subscribe.TopicName = slice("TEST/TIMEefghijk");
    subscribe.UserKeyVal[0] = slice("key1");
    subscribe.UserKeyVal[1] = slice("val1");
    subscribe.UserKeyVal[2] = slice("key2");
    subscribe.UserKeyVal[3] = slice("val2");
    subscribe.QoS = 1;
    subscribe.packetType = CtrlSubscribe;

    mqttBuffer1024 buffer;
    sink assemblyBuffer = buffer.getSink();

    mqttBuffer1024 result;
    SinkDrain netDrain;
    netDrain.buffer = result.getSink();

    bool ok = subscribe.outputPubOrSub(assemblyBuffer, &netDrain);

    slice rsl = netDrain.buffer.getWritten();
    char tmpbuffer[1024];

    //rsl.printhex();

    // should be:
    const char *need = "823000021a2600046b657931000476616c312600046b657932000476616c320010544553542f54494d4565666768696a6b01";
    const char *got = rsl.gethexstr(tmpbuffer,1024);
    if (strcmp(got, need) != 0)
    {
        cout << "testGenerateSubscribe FAIL: expected --" << need << "-- got --" << got << "--\n";
    }
}

void testParsePublish()
{
    const char *pubbytes = "3268000d544553542f54494d456162636400032d080010544553542f54494d4565666768696a6b2600046b657931000476616c312600046b657932000476616c326d65737361676520617420323032302d30332d32372030313a33353a33372e34303330373920633d31";

    using namespace mqtt5nano;

    mqttBuffer1024 testbuffer;

    slice net;
    net = testbuffer.loadHexString(pubbytes);
        // slice body = net ;//parserBuffer.loadFromFount(*net, len);
        // body.end = body.start + len;
        // net.start += len;

    char tmp[256];
    
    char * str = net.gethexstr(tmp,256);
   // cout << "loaded" << str << str << "\n";

    cout << "sizeof(mqttPacketPieces)=" << sizeof(mqttPacketPieces) << " ,\n"; // 200, 128 of which is the user key-values
    cout << "sizeof(UserKeyVal[8])=" << sizeof(slice[8]) << " ,\n"; // 128

    unsigned char packetType = net.readByte();
    int len = net.getLittleEndianVarLenInt();

    // now we can read the rest
    //mqttBuffer1024 parserBuffer;
        slice body = net ;//parserBuffer.loadFromFount(*net, len);
        body.end = body.start + len;
        net.start += len;

    mqttPacketPieces parser;
    bool failed = parser.parse(body, packetType, len);

    if ( failed ){
        cout << "parse result was fail=" << failed << "\n";
    } else {
        cout << "parse result ok\n";
    }

    // because .... it's a unit test.
    bool matched;
    const char *expected;
    char buffer[1024];
    matched = parser.TopicName.equals(expected = "TEST/TIMEabcd");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.TopicName.getCstr(buffer,1024) << "\n";
    }
    matched = parser.Payload.equals(expected = "message at 2020-03-27 01:35:37.403079 c=1");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.Payload.getCstr(buffer,1024) << "\n";
    }
    matched = parser.RespTopic.equals(expected = "TEST/TIMEefghijk");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.RespTopic.getCstr(buffer,1024) << "\n";
    }

    int i = 0;
    matched = parser.UserKeyVal[i].equals(expected = "key1");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i].getCstr(buffer,1024) << "\n";
    }
    matched = parser.UserKeyVal[i + 1].equals(expected = "val1");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i + 1].getCstr(buffer,1024) << "\n";
    }
    i = 2;
    matched = parser.UserKeyVal[i].equals(expected = "key2");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i].getCstr(buffer,1024) << "\n";
    }
    matched = parser.UserKeyVal[i + 1].equals(expected = "val2");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i + 1].getCstr(buffer,1024) << "\n";
    }
    i = 4;
    matched = parser.UserKeyVal[i].equals(expected = "");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i].getCstr(buffer,1024) << "\n";
    }
    matched = parser.UserKeyVal[i + 1].equals(expected = "");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i + 1].getCstr(buffer,1024) << "\n";
    }

    cout << "testParsePublish finished";
}


void testParsePublish2()
{
    const char *pubbytes = "306600213d78484b4c487630794e5f3738344c472d736d4b363475583679324d384e6c46642808000120260006787864656267000c5b787831323334353637385d260003617477000574657374316d736723636c69656e7449642d777331333175316577745f3432";

    using namespace mqtt5nano;

    mqttBuffer1024 testbuffer;

    slice net;
    net = testbuffer.loadHexString(pubbytes);

    char tmp[2560];
    
    // char * str = net.src.gethexstr(tmp,2560);
    // cout << "loaded" << str << str << "\n";

    cout << "sizeof(mqttPacketPieces)=" << sizeof(mqttPacketPieces) << " ,\n"; // 200, 128 of which is the user key-values
    cout << "sizeof(UserKeyVal[8])=" << sizeof(slice[8]) << " ,\n"; // 128

    unsigned char packetType = net.readByte();
    int len = net.getLittleEndianVarLenInt();

    // now we can read the rest
    mqttBuffer1024 parserBuffer;
    slice body = net;// parserBuffer.loadFromFount(net, len);
    body.end = body.start + len;
    net.start += len;

    mqttPacketPieces parser;
    bool failed = parser.parse(body, packetType, len);

    if ( failed ){
        cout << "parse result was fail=" << failed << "\n";
    } else {
        cout << "parse result ok\n";
    }

    // because .... it's a unit test.
    bool matched;
    const char *expected;
    char buffer[1024];
    matched = parser.TopicName.equals(expected = "=xHKLHv0yN_784LG-smK64uX6y2M8NlFd");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.TopicName.getCstr(buffer,1024) << "\n";
    }
    matched = parser.Payload.equals(expected = "msg#clientId-ws131u1ewt_42");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.Payload.getCstr(buffer,1024) << "\n";
    }
    matched = parser.RespTopic.equals(expected = " ");// what the hell?
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.RespTopic.getCstr(buffer,1024) << "\n";
    }

    int i = 0;
    matched = parser.UserKeyVal[i].equals(expected = "xxdebg");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i].getCstr(buffer,1024) << "\n";
    }
    matched = parser.UserKeyVal[i + 1].equals(expected = "[xx12345678]");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i + 1].getCstr(buffer,1024) << "\n";
    }
    i = 2;
    matched = parser.UserKeyVal[i].equals(expected = "atw");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i].getCstr(buffer,1024) << "\n";
    }
    matched = parser.UserKeyVal[i + 1].equals(expected = "test1");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i + 1].getCstr(buffer,1024) << "\n";
    }
    i = 4;
    matched = parser.UserKeyVal[i].equals(expected = "");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i].getCstr(buffer,1024) << "\n";
    }
    matched = parser.UserKeyVal[i + 1].equals(expected = "");
    if (!matched)
    {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i + 1].getCstr(buffer,1024) << "\n";
    }

    cout << "testParsePublish2 finished";
}

void testendians(){

    int val = 444;
    // test big endian
    // test big endian
    // test big endian
    char buff[4];
    char buff2[4];
    buff[0] = (val >> 7) + 0x80;
    buff[1] = val & 0x7F;

    slice s(buff);
    int test = s.getBigEndianVarLenInt();
    if ( test != val){
        cout << "FAIL getBigEndianVarLenInt\n";
    }
    sink d(buff2,4);
    d.writeBigEndianVarLenInt(val);
    if ( buff[0] != buff2[0] || buff[1] != buff2[1])
        cout << "FAIL writeBigEndianVarLenInt\n";
    
    // test litle endian 
    // test litle endian 
    // test litle endian 
    buff[0] = (val & 0x7F) + 0x80;
    buff[1] = (val>>7);

    s.start = 0;// reset s
     test = s.getLittleEndianVarLenInt();
    if ( test != val){
        cout << "FAIL getLittleEndianVarLenInt\n";
    }
    d.start = 0;// reset
    d.writeLittleEndianVarLenInt(val);
    if ( buff[0] != buff2[0] || buff[1] != buff2[1])
        cout << "FAIL writeLittleEndianVarLenInt\n";
    // todo: test 3 byte cases just in case.
    // this is arduino and we don't really expect anything to ever
    // be longer than 16k

}