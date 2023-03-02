#pragma once

#include "badjson.h"
#include "eepromItem.h"
#include "httpConverter.h"
#include "layers.h"
#include "mockWiFi.h"
#include "mqtt5nanoParse.h"
#include "nanobase64.h"
#include "streamReader.h"
#include "timedItem.h"
#include "wiFiCommands.h"

namespace mqtt5nano {

    /** Open a tcp socket (we're encrypting so tls is less necessary).
     * Send it a connect and then a subscribe.
     * Start collecting bytes in a buffer.
     * After we have a complete mqtt5 packet then try to extract a command out of
     * it and send the command to the usual command handlers.
     */

    extern EepromItem topicStash;
    extern EepromItem tokenStash;
    extern EepromItem adminPublicKeyStash;
    extern EepromItem devicePublicKeyStash;
    extern char topic[64];
    extern char passWord[512];

    struct MqttCommandClient {

        const char *host = "knotfree.io"; // fixme, add command to set this.
        // const char *host = "192.168.86.31"; // fixme
        int port = 1883;

        const char *clientId = "91sgEQGyqQSy8RBBB"; // fixme, add command to set this.
        const char *userName = "fG1EjiVTneOCHT5FU"; // fixme, add command to set this.
                                                    //  const char *passWord = "eyJhbGciOiJFZERTQSIsInR5cCI6IkpXVCJ9.eyJleHAiOjE2NzE5MjQyNDMsImlzcyI6Il85c2giLCJqdGkiOiJMWGR3UkQ5TVBuVTlHcWJyVUVxYWFINmciLCJpbiI6MTUyLCJvdXQiOjE1Miwic3UiOjEwMCwiY28iOjQsInVybCI6Imtub3RmcmVlLm5ldCJ9.vNebYTwk2agVkKAXdrnSDhXW1MDrTuZRDxYWBMJp2F1iyfVGT9KHvP0H2wVA0UGu6Ft0cx8cbdbleFFMLvQdDA";
                                                    //  const char *topic = "woot-merida-scroller-"; // "get-unix-time" or "backyard-temp-9gmf97inj5e";
        bool eeIsRead = false;

        bool clientconnected = false;
        WiFiClient client;
        bool sentConnect = false;
        bool sentSubscribe = false;

        static const int buffSize = 2048; // should be enough.? it's huge.
        char sinkbuffer[buffSize];
        ByteCollector mqttsink; // we collect the incoming bytes here until it's a whole packet.

        // we re-subscribe every 18 minutes. This also adjusts the time.
        struct subTimer : TimedItem {
            MqttCommandClient *cli;
            subTimer() {
                SetInterval(1000 * 60 * 18); // 18 min in ms
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

        void reset() { // start all over again. for when we're really screwed.
            static const char PROGMEM tmp[] = "# resetPM";
            // globalSerial->println(tmp);
            clientconnected = false;
            client.stop();
            sentConnect = false;
            sentSubscribe = false;
            backoff = 10000;
        }

        bool sendConnect() {
            char *buffer = sinkbuffer; // [1024];
            ByteCollector assemblyBufferSink(buffer, buffSize);

            mqttPacketPieces mqtt;
            StreamDestination socket(client);

            bool ok = mqtt.outputConnect(assemblyBufferSink, &socket, clientId, userName, passWord);
            return ok;
        }

        bool sendSubscribe(slice topic) {
            char buffer[128]; // limit of topic name is 128 bytes.
            ByteCollector assemblyBufferSink(buffer, 127);

            mqttPacketPieces subscribe;
            subscribe.PacketID = 2;
            subscribe.TopicName = slice(topic);
            subscribe.QoS = 1;
            subscribe.packetType = CtrlSubscribe;
            StreamDestination socket(client);
            bool ok = subscribe.outputPubOrSub(assemblyBufferSink, &socket);
            return ok;
        }

        long lastNow = 0;
        int backoff = 0;

        void loop(long now, class Stream &serial) {
            // haveStream = &serial;
            long delta = now - lastNow;
            lastNow = now;
            if (backoff > 0) {
                backoff -= delta;
                return;
            }
            bigloop(now); // todo break bigloop down more into smaller pieces.
        }

        void bigloop(long now) {

            if (!eeIsRead) { // could we just not do this?
                ByteDestination tmp(topic, sizeof(topic));
                topicStash.read(tmp);
                ByteDestination tmp2(passWord, sizeof(passWord));
                tokenStash.read(tmp2);

                globalSerial->println("# read topic, token");
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

                    globalSerial->print("# client.connect ");
                    globalSerial->print(host);
                    globalSerial->print(" ");
                    globalSerial->println(port);

                    // IPAddress ip;
                    // ip.fromString(host);// for when host is dotted quad

                    bool ok = client.connect(host, port); // timeout in ms
                    if (ok) {
                        globalSerial->println("# mqtt.connect ok ");
                        clientconnected = true;
                    } else {
                        globalSerial->println("# mqtt.connect FAIL ");
                        backoff = 10000;
                    }

                    return; // come back on the next pass
                }
                if (!sentConnect) {

                    globalSerial->println("# send mqtt Connect");

                    bool ok = sendConnect();
                    if (ok) {
                        sentConnect = true;
                        globalSerial->println("# sent mqtt connect ok");
                    } else {
                        globalSerial->println("# sent mqtt connect FAIL");
                        backoff = 1000;
                    }
                    // do we have to wait for the conack that I don't really care about?
                    return;
                }
                if (!sentSubscribe) {

                    bool ok = sendSubscribe(topic);
                    if (ok) {
                        sentSubscribe = true;
                        globalSerial->println("# sent subscribe ok");
                    } else {
                        globalSerial->println("# sent subscribe FAIL");
                        backoff = 1000;
                    }
                    // do we have to wait for the conack that I don't really care about.
                    return;
                }
                // if we're here we're supposed to be good.
                // globalSerial->println("# check is connected?");
                if (!client.connected()) {
                    globalSerial->println("# client not connected");
                    reset(); // what we do when we're not good.
                    backoff = 10000;
                    return;
                }
                // globalSerial->println("# have connected");
                // while (client.available())
                if (client.available()) {

                top:
                    char c = client.read();
                    mqttsink.writeByte(c);
                    if (mqttsink.remaining() < 2) {
                        globalSerial->println("# mqttsink overflow");
                        reset();
                        return;
                    }
                    slice position(mqttsink);
                    if (position.size() < 2) {
                        return;
                    }
                    int posStart = position.start;
                    unsigned char rawpacketType = position.readByte();
                    const int len = position.getLittleEndianVarLenInt();
                    if (len == -1) { // not enough bytes fail
                        return;
                    }
                    if (position.size() < len) { // there's not enough bytes for this packet
                        if (!client.available()) {
                            return; // try again next time.
                        }
                        goto top;
                    }

                    if (0) { // dump the packet in hex
                        globalSerial->println("# have mqtt packet in hex");
                        globalSerial->println(len);
                        char *hexDumpBuffer = (char *)malloc(len * 2 + 10);
                        int hexlen = hex::encode(position.base + posStart, position.end - posStart, hexDumpBuffer, len * 2 + 10);
                        hexDumpBuffer[hexlen] = 0;
                        globalSerial->println(hexDumpBuffer);
                        free(hexDumpBuffer);
                    }

                    {
                        int packetType = (rawpacketType >> 4);
                        int QoS = (rawpacketType >> 1) & 3; // unused

                        mqttPacketPieces parser;

                        bool ok = parser.parse(position, rawpacketType, len);
                        if (!ok) { // I don't know that we can recover from this.
                            globalSerial->println("# FAIL in parse ");
                            char hbuf[128];
                            globalSerial->println(packetType);
                            globalSerial->println(slice(mqttsink).gethexstr(hbuf, 128));
                            // bail out
                            reset(); // we need a whole new connection.
                            backoff = 100;
                            return;
                        }
                        // we're good
                        if (packetType == CtrlPublish) {
                            slice receivedTopic = parser.TopicName;
                            slice returnAddress = parser.RespTopic;
                            slice payload = parser.Payload;
                            {
                                CommandPipeline * pipeline = new CommandPipeline();
                                pipeline->isMqtt = true;
                                // user props etc.
                                // let's transfer the user props to the http props.
                                for (int i = 0; i < mqttPacketPieces::userKeyValLen; i++) {
                                    slice kv = parser.UserKeyVal[i];
                                    if (kv.empty()) {
                                        break;
                                    }
                                    badjson::Segment *seg;
                                    if (kv.startsWith("=")) {
                                        seg = new badjson::Base64Bytes(); // does this ever happen?
                                    } else if (kv.startsWith("$")) {
                                        seg = new badjson::HexBytes(); // does this ever happen?
                                    } else {
                                        badjson::RuneArray *ra = new badjson::RuneArray();
                                        ra->theQuote = 0; // it might just be straight bytes and not runes.
                                        ra->hadQuoteOrSlash = false;
                                        seg = ra;
                                    }
                                    seg->input = kv;
                                    pipeline->linkToTail(*seg);
                                }
                                pipeline->linkFrontToParams();
                                slice message = parser.Payload;

                                StreamDestination socket(client);

                                pipeline->parser = &parser;
                                pipeline->handlePayload(payload, socket);
                                // globalSerial->println("pipeline returns");
                                delete pipeline;
                                //  globalSerial->println("mqtt deleted pipeline");
                            } // and delete the pipeline


                                                            // if(0){ // show the packet payload
                                //     // globalSerial->println("publish packet:");
                                //     char * tmp = new char[payload.size()+1];
                                //     // globalSerial->println(receivedTopic.getCstr(tmp, sizeof(tmp)));
                                //     // globalSerial->println(returnAddress.getCstr(tmp, sizeof(tmp)));

                                //     globalSerial->println("show the payload");
                                //     globalSerial->println(payload.getCstr(tmp, payload.size()+1)); // show the payload
                                //     delete []tmp;
                                // }
                                // now we should just send it to the pipeline
                                // which will.

                                // char *cmdreplybuffer = new char[2048]; // FIXME: kinda big

                                // SinkDrain response(cmdreplybuffer, 2048);


                            // bool wasHttp = httpprops.convert(payload);
                            // if (wasHttp) {
                            //     // fixme:
                            //     httpprops.isHttP = true;
                            //     httpprops.haveSomething(payload)

                            // } else
                            // {
                            //     // chop up the payload as text
                            //     badjson::ResultsTriplette chopped = badjson::Chop(payload.base + payload.start, payload.size());
                            //     if (chopped.error != nullptr) {
                            //         // return a message ??
                            //         globalSerial->println("ERROR payload chop");
                            //         reset();
                            //         return;
                            //     }
                            //     httpprops.linkToTail(*chopped.segment); // httpprops will free the segments now
                            //     httpprops.linkFrontToCommand();
                            // }
                            // feed it to the commands.
                            // char *cmdreplybuffer = new char[2048]; // FIXME: kinda big

                            // SinkDrain response(cmdreplybuffer, 2048);
                            // Command::process(httpprops.command, httpprops.command, response);
                            // // now, for the response into a publish
                            // // wrap in a http reply if necessary.
                            // if (wasHttp) {
                            //     // fixme
                            //     // call http services to wrap output in http reply
                            //     // SinkDrain response = httpServide::wrap(response)
                            //     // response = response2;
                            // }

                            // globalSerial->print("reply is ");
                            // response.buffer.base[response.buffer.start] = 0;
                            // globalSerial->println(response.buffer.base);

                            //                         mqttPacketPieces pubgen;

                            //                         pubgen.QoS = 1;
                            //                         pubgen.packetType = CtrlPublish;

                            //                         pubgen.TopicName = parser.RespTopic;
                            //                         pubgen.PacketID = 3;
                            //                         pubgen.RespTopic = parser.TopicName;

                            //    // fixme                     pubgen.Payload = slice(response.buffer.base, 0, response.buffer.start);

                            //                         // copy the user props, from the pipeline.
                            //                         badjson::Segment *pP = pipeline.params;
                            //                         while (pP != nullptr) {
                            //                             pP = pP->next;
                            //                         }
                            //                         // for (int i = 0; i < parser.userKeyValLen; i++) {
                            //                         //     pubgen.UserKeyVal[i] = parser.UserKeyVal[i];
                            //                         // }

                            //                         // move this to the layers gadget?
                            //                         char assemblybuffer[1024]; // ?? how much 4096? new //FIXME: too big for stack
                            //                         ByteCollector assemblyArea(assemblybuffer, 1024);

                            //                       //  StreamDestination socket(client);

                            //                         bool ok = pubgen.outputPubOrSub(assemblyArea, &socket);
                            //                         if (!ok) {
                            //                             globalSerial->println("ERROR pubgen not ok");
                            //                             reset();
                            //                         }
                            mqttsink.reset();
                            // delete[] cmdreplybuffer;

                        } else if (packetType == CtrlConnAck) {
                            globalSerial->println("# CtrlConnAck");
                            mqttsink.reset();
                        } else if (packetType == CtrlSubAck) {
                            globalSerial->println("# CtrlSubAck");
                            // we can get the time from the CtrlSubAck
                            slice timeStr = parser.userKeyValueGet("unix-time");
                            // char tmpBuffer[64];
                            // timeStr.getCstr(tmpBuffer, 64);
                            // globalSerial->print("# timeStr ");
                            // globalSerial->println(tmpBuffer);
                            long t = timeStr.toLong();
                            // globalSerial->print("# time long ");
                            // globalSerial->println(t);
                            if (t > 0) {
                                millisUnixAdjust = (long long)t * 1000 - latestNowMillis;
                            }
                            // globalSerial->print("# unix time ");
                            // globalSerial->println(getUnixTime());
                            // globalSerial->print(sizeof(long long)); // long is just 4, need 8 for this
                            // globalSerial->print(sizeof(int));       // long is just 4
                            mqttsink.reset();
                        } else {
                            globalSerial->print("# mqtt unhandled type ");
                            globalSerial->println(packetType);
                            // reset the buffer to pass the command.
                            mqttsink.reset();
                        }
                    }
                }
            } else { // else not wifi connected
            }
        }

        // struct getAdminHint : Command {
        //     void init() override {
        //         name = "get admin hint";
        //         description = "first bytes from any admin keys we accept.🔓";
        //         argumentCount = 1;
        //     }
        //     void execute(Args args, badjson::Segment *params, drain &out) override {

        //         out.write("aaa bbb ccc");
        //     }
        // } getAdminHint;

        // struct getPubk : Command {
        //     void init() override {
        //         name = "get pubk";
        //         description = "public key of this thing.🔓";
        //         argumentCount = 1;
        //     }
        //     void execute(Args args, badjson::Segment *params, drain &out) override {

        //     }
        // } getPubk;

        struct topicGet : Command {
            void init() override {
                name = "get long name";
                description = "long name is unique over the world.";
            }
            void execute(Args args, badjson::Segment *params, Destination &out) override {
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
            void execute(Args args, badjson::Segment *params, Destination &out) override {
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
                description = "returns 'claims' from token.";
            }
            void execute(Args args, badjson::Segment *params, Destination &out) override {
                char buffer[tokenStash.size];
                ByteDestination bd(&buffer[0], tokenStash.size);
                tokenStash.read(bd);
                slice token(bd.buffer);
                // serialDestination.print("token ",token,"\n");
                int firstPeriod = token.indexOf('.');
                token.start = firstPeriod + 1;
                int secondPeriod = token.indexOf('.');
                token.end = token.start+secondPeriod;
                // serialDestination.print("token chopped",token,"\n");
                bd.reset();
                token.b64Decode(&bd.buffer);
                out.write(bd.buffer);
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
            void execute(Args args, badjson::Segment *params, Destination &out) override {
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
