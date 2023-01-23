#pragma once

#include "badjson.h"
#include "eepromItem.h"
#include "httpConverter.h"
#include "mockWiFi.h"
#include "mqtt5nanoParse.h"
#include "streamReader.h"
#include "timedItem.h"
#include "wiFiCommands.h"

namespace mqtt5nano {

    /** Open a tcp socket (or, better todo: a tls socket ).
     * Send it a connect and then a subscribe.
     * Start collecting bytes in a buffer.
     * After we have a complete mqtt5 packet then try to extract a command out of
     * it and send the command to the usual command handlers.
     */

    extern EepromItem topicStash;
    extern EepromItem tokenStash;
    extern char topic[64];
    extern char passWord[512];

    struct MqttCommandClient {

        const char *host = "knotfree.io"; // "192.168.86.23"; //"knotfree.net";
        int port = 1883;
        const char *clientId = "91sgEQGyqQSy8RBBB"; // fixme
        const char *userName = "fG1EjiVTneOCHT5FU"; // fixme
                                                    //  const char *passWord = "eyJhbGciOiJFZERTQSIsInR5cCI6IkpXVCJ9.eyJleHAiOjE2NzE5MjQyNDMsImlzcyI6Il85c2giLCJqdGkiOiJMWGR3UkQ5TVBuVTlHcWJyVUVxYWFINmciLCJpbiI6MTUyLCJvdXQiOjE1Miwic3UiOjEwMCwiY28iOjQsInVybCI6Imtub3RmcmVlLm5ldCJ9.vNebYTwk2agVkKAXdrnSDhXW1MDrTuZRDxYWBMJp2F1iyfVGT9KHvP0H2wVA0UGu6Ft0cx8cbdbleFFMLvQdDA";
                                                    //  const char *topic = "woot-merida-banner2"; // "get-unix-time" or "backyard-temp-9gmf97inj5e";
        bool eeIsRead = false;

        bool clientconnected = false;
        WiFiClient client;
        bool sentConnect = false;
        bool sentSubscribe = false;

        static const int buffSize = 4096; // should be enough.? it's huge
        char sinkbuffer[buffSize];
        sink mqttsink; // we collect the incoming bytes here until it's a whole packet.

        class Stream *haveStream;

        struct subTimer : TimedItem {
            MqttCommandClient *cli;
            subTimer() {
                SetInterval(1000 * 60 * 20); // 20 min in ms
            }
            void execute() override {
                if (cli->sentConnect) {
                    // send a subscribe
                    bool ok = cli->sendSubscribe(topic);
                    if (!ok) {
                        // println ?
                        cli->reset();
                    }
                }
            }
        };
        subTimer pingTimer;

        MqttCommandClient() : topicSet(this), tokenSet(this) {
            mqttsink.base = sinkbuffer;
            mqttsink.start = 0;
            mqttsink.end = buffSize;
            pingTimer.cli = this;
        }

        void reset() { // start all over again;
            static const char PROGMEM tmp[] = "# resetPM";
            //haveStream->println("# reset");
            clientconnected = false;
            client.stop();
            sentConnect = false;
            sentSubscribe = false;
            backoff = 10000;
            haveStream->println(tmp);
        }

        bool sendConnect() {
            char *buffer = sinkbuffer; // [1024];
            sink assemblyBufferSink(buffer, buffSize);

            mqttPacketPieces mqtt;
            StreamDrain socket(client);

            bool ok = mqtt.outputConnect(assemblyBufferSink, &socket, clientId, userName, passWord);
            return ok;
        }

        bool sendSubscribe(slice topic) {
            char buffer[128];
            sink assemblyBufferSink(buffer, 127);

            mqttPacketPieces subscribe;
            subscribe.PacketID = 2;
            subscribe.TopicName = slice(topic);
            subscribe.QoS = 1;
            subscribe.packetType = CtrlSubscribe;
            StreamDrain socket(client);
            bool ok = subscribe.outputPubOrSub(assemblyBufferSink, &socket);
            return ok;
        }

        long lastNow = 0;
        int backoff = 0;

        void loop(long now, class Stream &serial) {
            haveStream = &serial;
            long delta = now - lastNow;
            lastNow = now;
            if (backoff > 0) {
                backoff -= delta;
                return;
            }
            bigloop(now, serial); // todo break bigloop down more into smaller pieces.
        }

        void bigloop(long now, class Stream &serial) {

            if (!eeIsRead) {
                SinkDrain tmp(topic, sizeof(topic));
                topicStash.read(tmp);
                SinkDrain tmp2(passWord, sizeof(passWord));
                tokenStash.read(tmp2);

                serial.println("# read topic, token");
                eeIsRead = true;
            }
            // our long name
            if (topic[0] == 0) {
                return;
            }
            // really a  jwt
            if (passWord[0] == 0) {
                return;
            }
            if (connected) // if the wifi connected
            {
                // open a tcp socket
                if (!clientconnected) {

                    serial.println("# client.connect");

                    bool ok = client.connect(host, port);
                    if (ok) {
                        serial.println("# mqtt.connect ok ");
                        clientconnected = true;
                    } else {
                        serial.println("# mqtt.connect FAIL ");
                        backoff = 10000;
                    }

                    return; // come back on the next pass
                }
                if (!sentConnect) {

                    serial.println("# send mqtt Connect");

                    bool ok = sendConnect();
                    if (ok) {
                        sentConnect = true;
                        serial.println("# sent mqtt connect ok");
                    } else {
                        serial.println("# sent mqtt connect FAIL");
                        backoff = 1000;
                    }
                    // do we have to wait for the conack that I don't really care about?
                    return;
                }
                if (!sentSubscribe) {

                    bool ok = sendSubscribe(topic);
                    if (ok) {
                        sentSubscribe = true;
                        serial.println("# sent subscribe ok");
                    } else {
                        serial.println("# sent subscribe FAIL");
                        backoff = 1000;
                    }
                    // do we have to wait for the conack that I don't really care about.
                    return;
                }
                // if we're here we're supposed to be good.
                // serial.println("# check is connected?");
                if (!client.connected()) {
                    serial.println("# client not connected");
                    reset(); // what we do when we're not good.
                    backoff = 10000;
                    return;
                }
                // serial.println("# have connected");
                // while (client.available())
                if (client.available()) {
                    char c = client.read();
                    mqttsink.writeByte(c);
                    if (mqttsink.remaining() < 2) {

                        serial.println("# mqttsink overflow");

                        reset();
                        return;
                    }

                    // char cc[1];
                    // char inhex[3];
                    // cc[0] = c;
                    // hex::encode((const unsigned char *)cc, 1, inhex, 2);
                    // inhex[2] = 0;
                    // serial.print("got byte ");
                    // serial.println((char *)inhex);

                    // do we have a whole mqtt packet?

                    slice position(mqttsink);

                    if (position.size() < 2) {
                        return;
                    }
                    unsigned char rawpacketType = position.readByte();
                    const int len = position.getLittleEndianVarLenInt();
                    if (len == -1) { // not enough bytes fail
                        return;
                    }
                    if (position.size() < len) { // there'serial not enough bytes in availableNow for this packet
                        return;
                    }
                    serial.println("# have mqtt packet");
                    serial.println(len);
                    // serial.print("# before segmentsAllocated ");
                    // serial.println(badjson::segmentsAllocated);
                    // serial.print("# before freeHeap ");
                    // serial.println(ESP.getFreeHeap());

                    {
                        int packetType = (rawpacketType >> 4);
                        int QoS = (rawpacketType >> 1) & 3; // unused

                        mqttPacketPieces parser;
                        bool ok = parser.parse(position, rawpacketType, len);
                        if (!ok) { // I don't know that we can recover from this.
                            serial.println("# FAIL in parse ");
                            char hbuf[128];
                            serial.println(packetType);
                            serial.println(slice(mqttsink).gethexstr(hbuf, 128));
                            // bail out
                            reset();
                            return;
                        }
                        // we're good
                        if (packetType == CtrlPublish) {
                            slice receivedTopic = parser.TopicName;
                            slice returnAddress = parser.RespTopic;
                            slice payload = parser.Payload;
                            ParsedHttp httpprops;
                            // user props etc.
                            for (int i = 0; i < mqttPacketPieces::userKeyValLen; i++) {
                                slice kv = parser.UserKeyVal[i];
                                if (kv.empty()) {
                                    break;
                                }
                                badjson::Segment *seg;
                                if (kv.startsWith("=")) {
                                    seg = new badjson::Base64Bytes();
                                } else if (kv.startsWith("$")) {
                                    seg = new badjson::HexBytes();
                                } else {
                                    badjson::RuneArray *ra = new badjson::RuneArray();
                                    ra->theQuote = 0; // it might just be straight bytes and not runes.
                                    ra->hadQuoteOrSlash = false;
                                    seg = ra;
                                }
                                seg->input = kv;
                                httpprops.linkToTail(*seg);
                            }
                            httpprops.linkFrontToParams();
                            slice message = parser.Payload;

                            // serial.println("publish packet:");
                            char tmp[256];
                            // serial.println(receivedTopic.getCstr(tmp, 256));
                            // serial.println(returnAddress.getCstr(tmp, 256));

                            serial.println(payload.getCstr(tmp, 256)); // show the command
                            serial.print(".");

                            // we need to check if the message is a GET
                            bool wasHttp = httpprops.convert(payload);
                            if (wasHttp) {
                                // nothing
                            } else {
                                // chop up the payload as text
                                badjson::ResultsTriplette chopped = badjson::Chop(payload.base + payload.start, payload.size());
                                if (chopped.error != nullptr) {
                                    // return a message ??
                                    serial.println("ERROR payload chop");
                                    reset();
                                    return;
                                }
                                httpprops.linkToTail(*chopped.segment); // httpprops will free the segments now
                                httpprops.linkFrontToCommand();
                            }
                            // feed it to the commands.
                            char *cmdreplybuffer = new char[2048]; // FIXME: kinda big

                            // StreamDrain dr(serial);
                            // badjson::ToString(*httpprops.command, dr);
                            // if (httpprops.params) {
                            //     badjson::ToString(*httpprops.params, dr);
                            // }

                            SinkDrain response(cmdreplybuffer, 2048);
                            Command::process(httpprops.command, httpprops.command, response);
                            // now, for the response into a publish
                            // wrap in a http reply if necessary.
                            if (wasHttp) {
                                // fixme
                                // call http services to wrap output in http reply
                                // SinkDrain response = httpServide::wrap(response)
                                // response = response2;
                            }

                            serial.print("reply is ");
                            serial.println(response.buffer.base);

                            // it'serial still a SinkDrain called response

                            mqttPacketPieces pubgen;

                            pubgen.QoS = 1;
                            pubgen.packetType = CtrlPublish;

                            pubgen.TopicName = parser.RespTopic;
                            pubgen.PacketID = 3;
                            pubgen.RespTopic = parser.TopicName;

                            pubgen.Payload = sink(response.buffer);

                            for (int i = 0; i < parser.userKeyValLen; i++) {
                                pubgen.UserKeyVal[i] = parser.UserKeyVal[i];
                            }

                            char assemblybuffer[2048]; // ?? how much 4096? new //FIXME: too big for stack
                            sink assemblyArea(assemblybuffer, 2048);

                            StreamDrain socket(client);

                            bool ok = pubgen.outputPubOrSub(assemblyArea, &socket);
                            if (!ok) {
                                serial.println("ERROR pubgen not ok");
                                reset();
                            }
                            mqttsink.reset();
                            delete[] cmdreplybuffer;

                            // serial.print("# after segmentsAllocated ");
                            // serial.println(badjson::segmentsAllocated);
                            // serial.print("# after freeHeap ");
                            // serial.println(ESP.getFreeHeap());

                        } else if (packetType == CtrlConnAck) {
                            // static const char PROGMEM tmp[] = "# CtrlConnAck";
                            serial.println("# CtrlConnAck");
                            mqttsink.reset();
                        } else if (packetType == CtrlSubAck) {
                            serial.println("# CtrlSubAck");
                            mqttsink.reset();
                        } else {
                            serial.print("# mqtt unhandled type ");
                            serial.println(packetType);
                            // reset the buffer to pass the command.
                            mqttsink.reset();
                        }
                    }
                    // serial.print("# after segmentsAllocated ");
                    // serial.println(badjson::segmentsAllocated);
                    // serial.print("# after freeHeap ");
                    // serial.println(ESP.getFreeHeap());
                }
            } else { // else not wifi connected
            }
        }

        struct getAdminHint : Command {
            void init() override {
                name = "get admin hint";
                description = "first bytes from any admin keys we accept";
                argumentCount = 1;
            }
            void execute(Args args, badjson::Segment *params, drain &out) override {

                out.write("aaa bbb ccc");
            }
        } getAdminHint;

        // struct getAbout : Command {
        //     void init() override {
        //         name = "about";
        //         description = "it's all about me";
        //         argumentCount = 1;
        //     }
        //     void execute(Args args, badjson::Segment *params, drain &out) override {

        //         out.write("Mqtt5nano v.0.1.0");
        //     }
        // } getAbout;

        struct getPubk : Command {
            void init() override {
                name = "get pubk";
                description = "public key of this thing";
                argumentCount = 1;
            }
            void execute(Args args, badjson::Segment *params, drain &out) override {

                out.write("-none-");
            }
        } getPubk;

        struct topicGet : Command {
            void init() override {
                name = "get long name";
                description = "long name is unique over the world.";
            }
            void execute(Args args, badjson::Segment *params, drain &out) override {
                topicStash.read(out);
            }
        } topicGet;

        struct topicSet : Command {

            MqttCommandClient *cli;
            topicSet(MqttCommandClient *_cli) {
                cli = _cli;
            }
            void init() override {
                name = "set long name";
                description = "set long name is unique over the world";
                argumentCount = 1;
            }
            void execute(Args args, badjson::Segment *params, drain &out) override {
                if (args[0].empty()) {
                    out.write("ERROR expected a value");
                    return;
                }
                out.write(args[0]);
                topicStash.write(args[0]);
                out.write("ok: ");
                topicStash.read(out);
                cli->reset();
                cli->eeIsRead = false;
            }
        } topicSet;

        struct tokenGet : Command {
            void init() override {
                name = "get token";
                description = "shows ** if you have a token.";
            }
            void execute(Args args, badjson::Segment *params, drain &out) override {
                if (passWord[0] != 0) {
                    out.write("********");
                }
                tokenStash.read(out);
            }
        } tokenGet;

        struct tokenSet : Command {

            MqttCommandClient *cli;
            tokenSet(MqttCommandClient *_cli) {
                cli = _cli;
            }

            void init() override {
                name = "set token";
                description = "set access token";
                argumentCount = 1;
            }
            void execute(Args args, badjson::Segment *params, drain &out) override {
                if (args[0].empty()) {
                    out.write("ERROR expected a value");
                    return;
                }
                tokenStash.write(args[0]);
                out.write("ok: ");
                tokenStash.read(out);
                cli->reset();
                cli->eeIsRead = false;
            }
        } tokenSet;
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
