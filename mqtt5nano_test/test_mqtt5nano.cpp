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
#include <string>
#include <vector>

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

void testParsePublish2(); // below
void testParsePublish3(); // below

void testendians();

int main() {

    cout << "Hello World!\n";

    // we need to parse a pub and generate a pub.
    // we need to be able to generate a sub and generate a connect.
    // we need to not barf, ever ever ever, when receiving garbage.
    using namespace mqtt5nano;

    testParsePublish3();
    testParsePublish2();

    testParseMisc();

    testGenerateConnect();

    testGeneratePublish();

    testGenerateSubscribe();

    testParsePublish();
}

using namespace mqtt5nano;

bool parseHelper(mqttPacketPieces &parser, mqttBuffer1024 &parserBuffer, slice net) {
    bool ok = false;

    while (!net.empty()) {
        unsigned char packetType = net.readByte();
        int len = net.getLittleEndianVarLenInt();

        if (len > 1024 - 4) {
            ok = false;
            return ok;
        }
        if (len == 0) // is legit
        {
            ok = false;
            return ok;
        }

        // now we can read the rest
        //  mqttBuffer parserBuffer;
        slice body = net; // parserBuffer.loadFromFount(*net, len);
        body.end = body.start + len;
        net.start += len;

        ok = parser.parse(body, packetType, len);
        if (!ok) {
            cout << "FAIL in parse test \n";
            return ok;
        }
    }
    return ok;
}

void testParseMisc() {
    using namespace mqtt5nano;

    mqttBuffer1024 testbuffer;   // to supply the fount
    mqttBuffer1024 parserBuffer; // for use by the parser.

    mqttPacketPieces parser; // the parser.
    slice net;

    {
        const char *subackbytes = "901b000018260009756e69782d74696d65000a31323334353637383930";

        net = testbuffer.loadHexString(subackbytes);

        bool ok; // = parseHelper(parser, parserBuffer, net);
        unsigned char packetType = net.readByte();
        int len = net.getLittleEndianVarLenInt();
        slice body = net; // parserBuffer.loadFromFount(*net, len);
        body.end = body.start + len;
        net.start += len;
        ok = ok = parser.parse(body, packetType, len);

        if (!ok) {
            cout << "FAIL of some kind \n";
        }
        slice val = parser.userKeyValueGet("unix-time");
        if (val.empty()) {
            cout << "FAIL to find unix-time \n";
        }
    }

    const char *pubbytes = "200600000322000a"; // this is NOT a pub

    net = testbuffer.loadHexString(pubbytes);

    bool ok = parseHelper(parser, parserBuffer, net);

    if (!ok) {
        cout << "FAIL of some kind \n";
    }

    pubbytes = "900400010000"; // not a pub

    net = testbuffer.loadHexString(pubbytes);

    ok = parseHelper(parser, parserBuffer, net);

    if (!ok) {
        cout << "FAIL of some kind \n";
    }

    pubbytes = "40020004";

    net = testbuffer.loadHexString(pubbytes);

    ok = parseHelper(parser, parserBuffer, net);

    if (!ok) {
        cout << "FAIL of some kind \n";
    }

    // now, all in a row.
    pubbytes = "200600000322000a90040001000040020004";

    net = testbuffer.loadHexString(pubbytes);

    ok = parseHelper(parser, parserBuffer, net);

    if (!ok) {
        cout << "FAIL of some kind \n";
    }
    // what is in it?

    cout << "testParseMisc finished\n";
}

void testGenerateConnect() {
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
    const char *got = rsl.gethexstr(tmpbuffer, 1024);
    if (strcmp(got, need) != 0) {
        cout << "testGenerateConnect FAIL: expected --" << need << "-- got --" << got << "--\n";
    }
}

void testGeneratePublish() {
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
    const char *got = rsl.gethexstr(tmpbuffer, 1024);
    if (strcmp(got, need) != 0) {
        cout << "testGeneratePublish FAIL: expected " << need << " got " << got << "\n";
    }
}

void testGenerateSubscribe() {

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

    // rsl.printhex();

    // should be:
    const char *need = "823000021a2600046b657931000476616c312600046b657932000476616c320010544553542f54494d4565666768696a6b01";
    const char *got = rsl.gethexstr(tmpbuffer, 1024);
    if (strcmp(got, need) != 0) {
        cout << "testGenerateSubscribe FAIL: expected --" << need << "-- got --" << got << "--\n";
    }
}

void testParsePublish() {
    const char *pubbytes = "3268000d544553542f54494d456162636400032d080010544553542f54494d4565666768696a6b2600046b657931000476616c312600046b657932000476616c326d65737361676520617420323032302d30332d32372030313a33353a33372e34303330373920633d31";

    using namespace mqtt5nano;

    mqttBuffer1024 testbuffer;

    slice net;
    net = testbuffer.loadHexString(pubbytes);
    // slice body = net ;//parserBuffer.loadFromFount(*net, len);
    // body.end = body.start + len;
    // net.start += len;

    char tmp[256];

    char *str = net.gethexstr(tmp, 256);
    // cout << "loaded" << str << str << "\n";

    cout << "sizeof(mqttPacketPieces)=" << sizeof(mqttPacketPieces) << " ,\n"; // 200, 128 of which is the user key-values
    cout << "sizeof(UserKeyVal[8])=" << sizeof(slice[8]) << " ,\n";            // 128

    unsigned char packetType = net.readByte();
    int len = net.getLittleEndianVarLenInt();

    // now we can read the rest
    // mqttBuffer1024 parserBuffer;
    slice body = net; // parserBuffer.loadFromFount(*net, len);
    body.end = body.start + len;
    net.start += len;

    mqttPacketPieces parser;
    bool ok = parser.parse(body, packetType, len);

    if (!ok) {
        cout << "parse result was fail=" << ok << "\n";
    } else {
        cout << "parse result ok\n";
    }

    // because .... it's a unit test.
    bool matched;
    const char *expected;
    char buffer[1024];
    matched = parser.TopicName.equals(expected = "TEST/TIMEabcd");
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.TopicName.getCstr(buffer, 1024) << "\n";
    }
    matched = parser.Payload.equals(expected = "message at 2020-03-27 01:35:37.403079 c=1");
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.Payload.getCstr(buffer, 1024) << "\n";
    }
    matched = parser.RespTopic.equals(expected = "TEST/TIMEefghijk");
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.RespTopic.getCstr(buffer, 1024) << "\n";
    }

    int i = 0;
    matched = parser.UserKeyVal[i].equals(expected = "key1");
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i].getCstr(buffer, 1024) << "\n";
    }
    matched = parser.UserKeyVal[i + 1].equals(expected = "val1");
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i + 1].getCstr(buffer, 1024) << "\n";
    }
    i = 2;
    matched = parser.UserKeyVal[i].equals(expected = "key2");
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i].getCstr(buffer, 1024) << "\n";
    }
    matched = parser.UserKeyVal[i + 1].equals(expected = "val2");
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i + 1].getCstr(buffer, 1024) << "\n";
    }
    i = 4;
    matched = parser.UserKeyVal[i].equals(expected = "");
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i].getCstr(buffer, 1024) << "\n";
    }
    matched = parser.UserKeyVal[i + 1].equals(expected = "");
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i + 1].getCstr(buffer, 1024) << "\n";
    }

    cout << "testParsePublish finished";
}

void testParsePublish2() {
    const char *pubbytes = "306600213d78484b4c487630794e5f3738344c472d736d4b363475583679324d384e6c46642808000120260006787864656267000c5b787831323334353637385d260003617477000574657374316d736723636c69656e7449642d777331333175316577745f3432";

    using namespace mqtt5nano;

    mqttBuffer1024 testbuffer;

    slice net;
    net = testbuffer.loadHexString(pubbytes);

    char tmp[2560];

    // char * str = net.src.gethexstr(tmp,2560);
    // cout << "loaded" << str << str << "\n";

    cout << "sizeof(mqttPacketPieces)=" << sizeof(mqttPacketPieces) << " ,\n"; // 200, 128 of which is the user key-values
    cout << "sizeof(UserKeyVal[8])=" << sizeof(slice[8]) << " ,\n";            // 128

    unsigned char packetType = net.readByte();
    int len = net.getLittleEndianVarLenInt();

    // now we can read the rest
    mqttBuffer1024 parserBuffer;
    slice body = net; // parserBuffer.loadFromFount(net, len);
    body.end = body.start + len;
    net.start += len;

    mqttPacketPieces parser;
    bool ok = parser.parse(body, packetType, len);

    if (!ok) {
        cout << "parse result was fail=" << "\n";
    } else {
        cout << "parse result ok\n";
    }

    // because .... it's a unit test.
    bool matched;
    const char *expected;
    char buffer[1024];
    matched = parser.TopicName.equals(expected = "=xHKLHv0yN_784LG-smK64uX6y2M8NlFd");
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.TopicName.getCstr(buffer, 1024) << "\n";
    }
    matched = parser.Payload.equals(expected = "msg#clientId-ws131u1ewt_42");
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.Payload.getCstr(buffer, 1024) << "\n";
    }
    matched = parser.RespTopic.equals(expected = " "); // what the hell?
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.RespTopic.getCstr(buffer, 1024) << "\n";
    }

    int i = 0;
    matched = parser.UserKeyVal[i].equals(expected = "xxdebg");
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i].getCstr(buffer, 1024) << "\n";
    }
    matched = parser.UserKeyVal[i + 1].equals(expected = "[xx12345678]");
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i + 1].getCstr(buffer, 1024) << "\n";
    }
    i = 2;
    matched = parser.UserKeyVal[i].equals(expected = "atw");
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i].getCstr(buffer, 1024) << "\n";
    }
    matched = parser.UserKeyVal[i + 1].equals(expected = "test1");
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i + 1].getCstr(buffer, 1024) << "\n";
    }
    i = 4;
    matched = parser.UserKeyVal[i].equals(expected = "");
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i].getCstr(buffer, 1024) << "\n";
    }
    matched = parser.UserKeyVal[i + 1].equals(expected = "");
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i + 1].getCstr(buffer, 1024) << "\n";
    }

    cout << "testParsePublish2 finished";
}

void testParsePublish3() {
    const char *pubbytes = "30b20600213d4a625a4b724e4e4239525566337771635535736f74344e4a794e70577a546874240800213d375368744f734c595962336e4262563733787a45395536494759436c5a565367474554202f6765742f73686f72742f6e616d6520485454502f312e310a582d466f727761726465642d466f723a2031302e34322e352e310d0a582d5265616c2d49703a2031302e34322e352e310d0a5365632d43682d55612d4d6f62696c653a203f300d0a5365632d43682d55613a20224368726f6d69756d223b763d22313130222c20224e6f742041284272616e64223b763d223234222c2022476f6f676c65204368726f6d65223b763d22313130220d0a5365632d46657463682d536974653a2063726f73732d736974650d0a582d466f727761726465642d506f72743a203434330d0a582d466f727761726465642d50726f746f3a2068747470730d0a557365722d4167656e743a204d6f7a696c6c612f352e3020284d6163696e746f73683b20496e74656c204d6163204f5320582031305f31355f3729204170706c655765624b69742f3533372e333620284b48544d4c2c206c696b65204765636b6f29204368726f6d652f3131302e302e302e30205361666172692f3533372e33360d0a526566657265723a20687474703a2f2f6c6f63616c686f73743a333030302f0d0a43616368652d436f6e74726f6c3a206e6f2d63616368650d0a4f726967696e3a20687474703a2f2f6c6f63616c686f73743a333030300d0a507261676d613a206e6f2d63616368650d0a582d466f727761726465642d5365727665723a207472616566696b2d376239633564666235382d37363962390d0a4163636570743a202a2f2a0d0a4163636570742d456e636f64696e673a20677a69702c206465666c6174652c2062720d0a5365632d46657463682d446573743a20656d7074790d0a5365632d46657463682d4d6f64653a20636f72730d0a582d466f727761726465642d486f73743a206174772d74657374312d7365727665722e6b6e6f74667265652e6e65740d0a4163636570742d4c616e67756167653a20656e2d55532c656e3b713d302e390d0a5365632d43682d55612d506c6174666f726d3a20226d61634f53220d0a0a";

    using namespace mqtt5nano;

    mqttBuffer1024 testbuffer;

    slice net;
    net = testbuffer.loadHexString(pubbytes);

    char tmp[2560];

    // char * str = net.src.gethexstr(tmp,2560);
    // cout << "loaded" << str << str << "\n";

    cout << "sizeof(mqttPacketPieces)=" << sizeof(mqttPacketPieces) << " ,\n"; // 200, 128 of which is the user key-values
    cout << "sizeof(UserKeyVal[8])=" << sizeof(slice[8]) << " ,\n";            // 128

    unsigned char packetType = net.readByte();
    int len = net.getLittleEndianVarLenInt();

    // now we can read the rest
    mqttBuffer1024 parserBuffer;
    slice body = net; // parserBuffer.loadFromFount(net, len);
    body.end = body.start + len;
    net.start += len;

    mqttPacketPieces parser;
    bool ok = parser.parse(body, packetType, len);

    if (!ok) {
        cout << "parse result was fail=" << "\n";
    } else {
        cout << "parse result ok\n";
    }

    // because .... it's a unit test.
    bool matched;
    const char *expected;
    char buffer[1024];
    matched = parser.TopicName.equals(expected = "=JbZKrNNB9RUf3wqcU5sot4NJyNpWzTht");
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.TopicName.getCstr(buffer, 1024) << "\n";
    }
    matched = parser.Payload.equals(expected = "a html get with lots of headers");
    if (!matched) {
       //  cout << "FAIL: expected " << expected << " got " << parser.Payload.getCstr(buffer, 1024) << "\n";
    }
    matched = parser.RespTopic.equals(expected = "=7ShtOsLYYb3nBbV73xzE9U6IGYClZVSg"); // what the hell?
    if (!matched) {
        cout << "FAIL: expected " << expected << " got " << parser.RespTopic.getCstr(buffer, 1024) << "\n";
    }

    // there are no user key values in this packet. 
    // int i = 0;
    // matched = parser.UserKeyVal[i].equals(expected = "xxdebg");
    // if (!matched) {
    //     cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i].getCstr(buffer, 1024) << "\n";
    // }
    // matched = parser.UserKeyVal[i + 1].equals(expected = "[xx12345678]");
    // if (!matched) {
    //     cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i + 1].getCstr(buffer, 1024) << "\n";
    // }
    // i = 2;
    // matched = parser.UserKeyVal[i].equals(expected = "atw");
    // if (!matched) {
    //     cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i].getCstr(buffer, 1024) << "\n";
    // }
    // matched = parser.UserKeyVal[i + 1].equals(expected = "test1");
    // if (!matched) {
    //     cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i + 1].getCstr(buffer, 1024) << "\n";
    // }
    // i = 4;
    // matched = parser.UserKeyVal[i].equals(expected = "");
    // if (!matched) {
    //     cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i].getCstr(buffer, 1024) << "\n";
    // }
    // matched = parser.UserKeyVal[i + 1].equals(expected = "");
    // if (!matched) {
    //     cout << "FAIL: expected " << expected << " got " << parser.UserKeyVal[i + 1].getCstr(buffer, 1024) << "\n";
    // }

    cout << "testParsePublish3 finished";
}


void testendians() {

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
    if (test != val) {
        cout << "FAIL getBigEndianVarLenInt\n";
    }
    sink d(buff2, 4);
    d.writeBigEndianVarLenInt(val);
    if (buff[0] != buff2[0] || buff[1] != buff2[1])
        cout << "FAIL writeBigEndianVarLenInt\n";

    // test litle endian
    // test litle endian
    // test litle endian
    buff[0] = (val & 0x7F) + 0x80;
    buff[1] = (val >> 7);

    s.start = 0; // reset s
    test = s.getLittleEndianVarLenInt();
    if (test != val) {
        cout << "FAIL getLittleEndianVarLenInt\n";
    }
    d.start = 0; // reset
    d.writeLittleEndianVarLenInt(val);
    if (buff[0] != buff2[0] || buff[1] != buff2[1])
        cout << "FAIL writeLittleEndianVarLenInt\n";
    // todo: test 3 byte cases just in case.
    // this is arduino and we don't really expect anything to ever
    // be longer than 16k
}