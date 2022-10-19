#pragma once

#include "wiFiCommands.h"
#include "knotWiFi.h"

#include "httpConverter.h"
#include "streamReader.h"

#include "mqtt5nano.h"

#include "knotbase64.h"

namespace mqtt5nano
{

    /** Open a tcp socket (or, better todo: a tls socket ).
     * Send it a connect and then a subscribe.
     * Start collecting bytes in a buffer.
     * After we have a complete mqtt5 packet then try to extract a command out of
     * it and send the command to the usual command handlers.
     */

    struct MqttCommandClient
    {
        // FIXME: these need to ALL be in eeprom
        const char *host = "knotfree.net";
        int port = 1883;
        const char *clientId = "";
        const char *userName = "";
        const char *passWord = "eyJhbGciOiJFZDI1NTE5IiwidHlwIjoiSldUIn0.eyJleHAiOjE2NjY3NDE4MzAsImlzcyI6Il85c2giLCJqdGkiOiIzV2hpYUl3ZDd6MF8tVVdvTHIyWGlPN3EiLCJpbiI6MTI4LCJvdXQiOjEyOCwic3UiOjIwLCJjbyI6MiwidXJsIjoia25vdGZyZWUubmV0In0.EktePKS8ll4IYT3HbzyZWfG1EjiVTneOCHT5FU1ulX5CyP5H_-v91sgEQGyqQSy8RBBBjvdqKgLDehgrve9BAQ";
        const char *topic = "backyard-temp-9gmf97inj5e";

        bool clientconnected = false;
        WiFiClient client;
        bool sentConnect = false;
        bool sentSubscribe = false;

        static const int buffSize = 4096; // should be enough.?
        char sinkbuffer[buffSize];
        sink mqttsink; // we collect the incoming bytes here until it's a whole packet.

        struct subTimer : TimedItem
        {
            MqttCommandClient *cli;
            subTimer()
            {
                interval = 20 * 60 * 1000; // 20 min in ms
            }
            void execute() override
            {
                if (cli->sentConnect)
                {
                    // send a subscribe
                    bool ok = cli->sendSubscribe(cli->topic);
                    if (!ok)
                    {
                        cli->reset();
                    }
                }
            }
        };
        subTimer pingTimer;

        MqttCommandClient()
        {
            mqttsink.base = sinkbuffer;
            mqttsink.start = 0;
            mqttsink.end = buffSize;
            pingTimer.cli = this;
        }

        void reset()
        { // start all over again;
            clientconnected = false;
            client.stop();
            sentConnect = false;
            sentSubscribe = false;
        }

        bool sendConnect()
        {
            char buffer[1024];
            sink assemblyBufferSink(buffer, 1024);

            mqttPacketPieces mqtt;
            StreamDrain socket(client);

            bool ok = mqtt.outputConnect(assemblyBufferSink, &socket, clientId, userName, passWord);
            return ok;
        }

        bool sendSubscribe(slice topic)
        {
            char buffer[1024];
            sink assemblyBufferSink(buffer, 1024);

            mqttPacketPieces subscribe;
            subscribe.PacketID = 2;
            // add props?
            subscribe.TopicName = slice(topic);
            // subscribe.UserKeyVal[0] = slice("key1");
            // subscribe.UserKeyVal[1] = slice("val1");
            // subscribe.UserKeyVal[2] = slice("key2");
            // subscribe.UserKeyVal[3] = slice("val2");
            subscribe.QoS = 1;
            subscribe.packetType = CtrlSubscribe;
            StreamDrain socket(client);
            bool ok = subscribe.outputPubOrSub(assemblyBufferSink, &socket);
            return ok;
        }

        void loop(long now, class Stream &s)
        {
            bigloop(now, s);
            pingTimer.loop(now);
            // todo break it down more
        }

        void bigloop(long now, class Stream &s)
        {
            if (connected) // if the wifi connected
            {
                // open a tcp socket
                if (!clientconnected)
                {
                    bool ok = client.connect(host, port);
                    if (ok)
                    {
                        s.println("client.connect ok ");
                        clientconnected = true;
                    }
                    else
                    {
                        s.println("client.connect FAIL ");
                    }
                    return; // come back on the next pass
                }
                if (!sentConnect)
                {
                    bool ok = sendConnect();
                    if (ok)
                    {
                        sentConnect = true;
                        s.println("sent connect ok");
                    }
                    else
                    {
                        s.println("sent connect FAIL");
                    }
                    // do we have to wait for the conack that I don't really care about.
                    return;
                }
                if (!sentSubscribe)
                {
                    bool ok = sendSubscribe(topic);
                    if (ok)
                    {
                        sentSubscribe = true;
                        s.println("sent subscribe ok");
                    }
                    else
                    {
                        s.println("sent subscribe FAIL");
                    }
                    // do we have to wait for the conack that I don't really care about.
                    return;
                }
                // if we're here we're supposed to be good.
                if (!client.connected())
                {
                    reset(); // what we do when we're not good.
                    return;
                }
                // while (client.available())
                if (client.available())
                {
                    char c = client.read();
                    mqttsink.writeByte(c);

                    // char cc[1];
                    // char inhex[3];
                    // cc[0] = c;
                    // hex::encode((const unsigned char *)cc, 1, inhex, 2);
                    // inhex[2] = 0;
                    // s.print("got byte ");
                    // s.println((char *)inhex);

                    // do we have a whole mqtt packet?

                    slice position(mqttsink);

                    if (position.size() < 2)
                    {
                        return;
                    }
                    unsigned char rawpacketType = position.readByte();
                    const int len = position.getLittleEndianVarLenInt();
                    if (len == -1)
                    { // not enough bytes fail
                        return;
                    }
                    if (position.size() < len)
                    { // there's not enough bytes in availableNow for this packet
                        return;
                    }
                    // make into procedure:
                    {
                        int packetType = (rawpacketType >> 4);
                        int QoS = (rawpacketType >> 1) & 3; // unused

                        mqttPacketPieces parser;
                        bool fail = parser.parse(position, rawpacketType, len);
                        if (fail)
                        { // I don't know that we can recover from this.
                            s.println("FAIL in parse ");
                            // bail out
                            reset();
                            return;
                        }
                        // we're good
                        if (packetType == CtrlPublish)
                        {
                            slice receivedTopic = parser.TopicName;
                            slice returnAddress = parser.RespTopic;
                            slice payload = parser.Payload;
                            ParsedHttp httpprops;
                            // user props etc.
                            for (int i = 0; i < mqttPacketPieces::userKeyValLen; i++)
                            {
                                slice kv = parser.UserKeyVal[i];
                                if (kv.empty())
                                {
                                    break;
                                }
                                badjson::Segment *seg;
                                if (kv.startsWith("="))
                                {
                                    seg = new badjson::Base64Bytes();
                                }
                                else if (kv.startsWith("$"))
                                {
                                    seg = new badjson::HexBytes();
                                }
                                else
                                {
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

                            s.println("publish packet:");
                            char tmp[256];
                            s.println(receivedTopic.getCstr(tmp, 256));
                            s.println(returnAddress.getCstr(tmp, 256));
                            s.println(payload.getCstr(tmp, 256));

                            // we need to check if the message is a GET
                            bool wasHttp = httpprops.convert(payload);
                            if (wasHttp)
                            {
                                // nothing
                            }
                            else
                            {
                                // chop up the payload as text
                                badjson::ResultsTriplette chopped = badjson::Chop(payload.base + payload.start, payload.size());
                                if (chopped.error != nullptr)
                                {
                                    // return a message ??
                                    s.println("got mqtt payload command parse error");
                                    reset();
                                    return;
                                }
                                httpprops.linkToTail(*chopped.segment);
                                httpprops.linkFrontToCommand();
                            }
                            // feed it to the commands.
                            char *cmdreplybuffer = new char[2048];

                            StreamDrain dr(s);
                            badjson::ToString(*httpprops.command, dr);
                            if (httpprops.params)
                            {
                                badjson::ToString(*httpprops.params, dr);
                            }

                            SinkDrain response(cmdreplybuffer, 2048);
                            process(httpprops.command, httpprops.command, response);
                            // now, for the response into a publish
                            // wrap in a http reply if necessary.
                            if (wasHttp)
                            {
                                // fixme
                                // call http services to wrap output in http reply
                                // SinkDrain response = httpServide::wrap(response)
                                // response = response2;
                            }

                            // it's still a SinkDrain called response

                            mqttPacketPieces pubgen;

                            pubgen.QoS = 1;
                            pubgen.packetType = CtrlPublish;

                            pubgen.TopicName = parser.RespTopic;
                            pubgen.PacketID = 3;
                            pubgen.RespTopic = parser.TopicName;

                            pubgen.Payload = sink(response.buffer);

                            char tmpbuffer[2048]; // ?? how much
                            sink assemblyArea(tmpbuffer, 2048);

                            StreamDrain socket(client);

                            bool ok = pubgen.outputPubOrSub(assemblyArea, &socket);
                            if (!ok)
                            {
                                s.println("Output pub not ok");
                                reset();
                            }
                            mqttsink.reset();
                        }
                        else if (packetType == CtrlConnAck)
                        {
                            mqttsink.reset();
                        }
                        else if (packetType == CtrlSubAck)
                        {
                            mqttsink.reset();
                        }
                        else
                        {
                            s.print("mqtt got unhandled packet type");
                            s.println(packetType);
                            // reset the buffer to pass the command.
                            mqttsink.reset();
                        }
                    }
                }
            }
            else
            { // else not wifi connected
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
