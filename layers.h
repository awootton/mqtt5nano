
#pragma once

#include "commandLine.h"
#include "eepromItem.h"
#include "httpConverter.h"
#include "mockStream.h"

#include "slices.h"

#include "commonCommands.h"
#include "mqtt5nanoParse.h"
#include "nanoCommon.h"
#include "nanoCrypto.h"
#include "streamReader.h"

// Pipeline is a series of routines to process a command.
// It is upside down from the pipeline in knotfree-net-homepage Pipeline.tsx

namespace mqtt5nano {

    // this is a return pipeline that returns http and/or mqtt with the result of a command.
    struct CommandPipeline : ParsedHttp {

        bool fromHttp = false; // the same as local?
        bool isHttP = false;
        bool isMqtt = false;
        bool isSerial = false;
        bool isLocal = false;
        bool isPng = false; // it could be http or it could be http in mqtt. local is unimplemented.
        bool wasEncrypted = false;

        bool wasHttp = false; // we found some http

        const int BufferLen = 2048;
        char *cmdreplybuffer = new char[BufferLen];
        char *cmdoutbuffer = new char[BufferLen];
        char *b64DecodeBytes = new char[BufferLen];

        ByteDestination response;        // the total response
        ByteDestination commandOut;      // just the output of the command.
        ByteDestination b64DecodeBuffer; // someplace to put the base64 decode.

        mqttPacketPieces *parser; // set by caller

        const char *errorString = nullptr;

        char adminpubkey[32];
        char privkey[32];
        char nonce[24];

        CommandPipeline() : response(cmdreplybuffer, BufferLen),
                            commandOut(cmdoutbuffer, BufferLen),
                            b64DecodeBuffer(b64DecodeBytes, BufferLen) {
        }

        ~CommandPipeline() {

            if (cmdreplybuffer != nullptr) {
                delete[] cmdreplybuffer;
                cmdreplybuffer = nullptr;
            }
            if (cmdoutbuffer != nullptr) {
                delete[] cmdoutbuffer;
                cmdoutbuffer = nullptr;
            }
            if (b64DecodeBytes != nullptr) {
                delete[] b64DecodeBytes;
                b64DecodeBytes = nullptr;
            }
        }

        slice b64Decode(slice s) {
            slice result = s.b64Decode(&b64DecodeBuffer.buffer);
            return result;
        }

        void handlePayload(slice payload, Destination &socket) {

            handlePayloadPart1(payload, socket);

            serialDestination.print("commandOut holds:",commandOut.buffer, "\n");

            if (wasHttp && !isMqtt) {
                // make it into a http reply
                if(1){
                    response.reset();
                    makeHttpReply(commandOut, response);
                    serialDestination.print("http http reply payload\n", response.buffer, "\n");
                }
                makeHttpReply(commandOut, socket); // doesn't use response

            } else if (wasHttp) {
                // http over mqtt
                response.reset();
                makeHttpReply(commandOut, response);
                // serialDestination.print("mqtt http reply payload\n", response.buffer, "\n");
                makeMqttReply(response, socket); // uses base64ByteDestination as temp.

                // serialDestination.print("mqtt done with socket", "\n");

            } else {
                // just mqtt
                makeMqttReply(commandOut, socket); // uses base64ByteDestination as temp.
            }

            // serialDestination.println("layers returning now");


        }

        // handlePayload does the whole job. TODO: break into parts.
        void handlePayloadPart1(slice payload, Destination &socket) {

            // serialDestination.println("have payload");
            wasHttp = convert(payload);
            if (wasHttp) {
                isHttP = true;
            } else {
                // is just the raw command line from mqtt (or serial?).
                {
                    if (command != nullptr) {
                        delete command;
                        command = nullptr;
                    }
                    // chop up the payload as text
                    badjson::ResultsTriplette chopped = badjson::Chop(payload.base + payload.start, payload.size());
                    if (chopped.error != nullptr) {
                        // return a message ??
                        errorString = "ERROR payload chop ";
                        serialDestination.print(errorString);
                        serialDestination.println(chopped.error);
                        commandOut.print(errorString);
                        commandOut.print(chopped.error);
                        return;
                    }
                    linkToTail(*chopped.segment); // httpprops will free the segments now
                    linkFrontToCommand();
                }
            }

            CommandSource source = unknown;
            if (isSerial) {
                source = CommandSource::SerialPort;
            } else if (wasHttp && isMqtt) {
                source = httpInMqtt;
            } else if (isMqtt) {
                source = CommandSource::Mqtt;
            } else if (wasHttp) {
                source = CommandSource::Local;
            } else {
                errorString = "ERROR unknown source";
                serialDestination.println(errorString);
    
                commandOut.print(errorString);
                return;// should never happen
            }

            // needs decryption if there's an admn key and a nonc key
            slice nonc = findParam("nonc");
            slice admn = findParam("admn");

            // serialDestination.print("command is ",command->input,"\n");

            bool isb64 = command->input.startsWith("=");
            if (isb64) {
                // decode into b64DecodeBuffer
                command->input.start++;// skip the =
                command->input = b64Decode(command->input);
            }

            if (nonc.size() > 0 && admn.size() > 0) {
                wasEncrypted = true;
                // serialDestination.println("decrypting");

                slice adminPublicKey64 = adminPublicKeyStash.readSlice(b64DecodeBuffer);
                b64DecodeBuffer.buffer.start += adminPublicKey64.size();
                slice devicePrivateKey64 = devicePrivateKeyStash.readSlice(b64DecodeBuffer);
                b64DecodeBuffer.buffer.start += devicePrivateKey64.size();
                slice devicePublicKey64 = devicePublicKeyStash.readSlice(b64DecodeBuffer);
                b64DecodeBuffer.buffer.start += devicePrivateKey64.size();

                if(0){
                serialDestination.print("nonc is ", nonc, "\n");
                serialDestination.print("admn is ", admn, "\n");
                serialDestination.print("adminPublicKey64 is ", adminPublicKey64, "\n");
                serialDestination.print("devicePrivateKey64 is ", devicePrivateKey64, "\n");
                }
                if (!adminPublicKey64.startsWith(admn)) {
                    errorString = "ERROR admn mismatch";
                    serialDestination.println(errorString);
                    commandOut.print(errorString);
                    return;
                }

                nonc.copy((char *)nonce, 24);
                ByteCollector dtmp = ByteCollector((char *)adminpubkey, 32);
                adminPublicKey64.b64Decode(&dtmp);
                ByteCollector dtmp2 = ByteCollector((char *)privkey, 32);
                devicePrivateKey64.b64Decode(&dtmp2);

                int decryptedStart = b64DecodeBuffer.buffer.start;
                ByteCollector *dP = &(b64DecodeBuffer.buffer);
                bool ok = nanocrypto::unbox(dP, command->input, nonce, adminpubkey, privkey);
                if (!ok) {
                    errorString = "ERROR crypto::unbox";
                    serialDestination.println(errorString);
                    commandOut.print(errorString);
                    return;
                }
                command->input = slice(b64DecodeBuffer.buffer.base, decryptedStart, b64DecodeBuffer.buffer.start);

                // serialDestination.print("clear command is ", command->input, "\n"); // ie version#1676856950

                int index = command->input.indexOf('#');
                if (index == -1) {
                    errorString = "ERROR no #";
                    serialDestination.println(errorString);
                    commandOut.print(errorString);
                    return;
                }

                // now split on #

                slice newCommand = command->input;
                slice timestr = command->input;
                newCommand.end = newCommand.start + index;
                timestr.start += index + 1;

                int time = timestr.toLong();
                int delta = time - getUnixTime();
                if (delta < 0) {
                    delta = -delta;
                }
                if (delta > 30) {
                    errorString = "ERROR time delta";
                    serialDestination.print(errorString);
                    serialDestination.writeInt(delta);
                    serialDestination.print("\n");
                    commandOut.print(errorString);
                    return;
                }

                { // re parse with badjson, from the b64DecodeBuffer
                    // chop up the payload as text

                    // first remove the '/'
                    int start = b64DecodeBuffer.buffer.start;
                    b64DecodeBuffer.write(newCommand); // copy the newCommand into the buffer
                    for (int i = start; i < b64DecodeBuffer.buffer.start; i++) {
                        if (b64DecodeBuffer.buffer.base[i] == '/') {
                            b64DecodeBuffer.buffer.base[i] = ' ';
                        }
                    }
                    newCommand = slice(b64DecodeBuffer.buffer.base, start, b64DecodeBuffer.buffer.start); // make a new slice
                    // serialDestination.print("pre chopped command is ", newCommand, "\n");

                    if (command != nullptr) {
                        delete command;
                        command = nullptr;
                    }
                    badjson::ResultsTriplette chopped = badjson::Chop(newCommand.base + newCommand.start, newCommand.size());
                    if (chopped.error != nullptr) {
                        // return a message ??
                        errorString = "ERROR payload chop2";
                        serialDestination.print(errorString, chopped.error, "\n");
                        commandOut.print(errorString);
                        return;
                    }

                    command = chopped.segment;
                }
            } else {
                // serialDestination.println("NOT decrypting");
            }

            // serialDestination.println("The executing command is: ");
            // badjson::ToString(*command, serialDestination);
            // serialDestination.println("\n");

            ByteDestination *commandDrain = &commandOut; // we output the command to the commandOut buffer
            if (wasEncrypted) {            
                response.reset();              // unless it's going to me encrypted then we output to the response buffer instead and then encrypt it to commandOut
                commandDrain = &response;
            }
            commandDrain->buffer.start = 0;
            slice processedCommand = commandDrain->buffer;

            processedCommand.start = commandDrain->buffer.start;
            Command::process(command, params, *commandDrain, source, wasEncrypted);
            if ( command->input.equals("favicon.ico") ) {
                isPng = true;// a hack
            }
            if (wasEncrypted) {
                commandDrain->write("#");
                commandDrain->writeInt(getUnixTime());
            }
            processedCommand.end = commandDrain->buffer.start;

            // serialDestination.print("processed command is: ", processedCommand, "\n");

            // if it was encrypted then now it needs to be encrypted again.
            if (wasEncrypted) {
                slice ourCommandResult = processedCommand;
                if (errorString != nullptr) {
                    ourCommandResult = slice(errorString);
                }

                commandOut.buffer.reset();
                b64DecodeBuffer.buffer.reset();
                // encrypt from response to b64DecodeBuffer
                ByteCollector *dP = &(b64DecodeBuffer.buffer);
                bool ok = nanocrypto::box(dP, ourCommandResult, nonce, adminpubkey, privkey);
                if (!ok) {
                    errorString = "ERROR crypto::box";
                    serialDestination.println("ERROR crypto::box");
                    commandOut.print(errorString);
                    return;
                }
                // now b64 from b64DecodeBuffer to commandOut
                commandOut.print("=");
                slice boxbinary(b64DecodeBuffer.buffer);
                boxbinary.b64Encode(&commandOut.buffer);

            }
        }

        bool makeHttpReply(ByteDestination src, Destination &dest) {

// fixme: make F work here
#if not defined(Arduino)
#define F(a) a
#endif
            int commandOutLen = src.buffer.start;

            dest.print(F("HTTP/1.1 200 OK\r\n"));
            dest.print(F("Content-Length: "));

            dest.print(commandOutLen);
            dest.print(F("\r\n"));
            if (isPng) { // hack alert
                dest.print(F("Content-Type: image/png\r\n"));
            } else {
                dest.print(F("Content-Type: text/plain\r\n"));
            }
            dest.print(F("Access-Control-Allow-Origin: *\r\n"));
            dest.print(F("Access-control-expose-headers: nonc\r\n"));
            dest.print(F("Connection: Closed\r\n"));
            dest.print(F("\r\n"));
            for (int i = 0; i < commandOutLen; i++) {
                dest.writeByte(src.buffer.base[i]);
            }
            return true; // ok
        }

        bool makeMqttReply(ByteDestination src, Destination &dest) {

            mqttPacketPieces pubgen;

            pubgen.QoS = 1;
            pubgen.packetType = CtrlPublish;

            pubgen.TopicName = parser->RespTopic;
            pubgen.PacketID = 3;
            pubgen.RespTopic = parser->TopicName;

            pubgen.Payload = slice(src.buffer.base, 0, src.buffer.start);

            // copy the user props, from the pipeline. Mostly to pick up the nonc 
            badjson::Segment *pP = params;
            int i = 0;
            while (pP != nullptr) {
                pubgen.UserKeyVal[i++] = pP->input;
                if (i >= pubgen.userKeyValLen) {
                    break;
                }
                pP = pP->next;
            }
            
            if(0){// print them out for debug
                for (int i = 0; i < pubgen.userKeyValLen; i++) {
                    if ( pubgen.UserKeyVal[i].empty() ) {
                       break;
                    }
                    serialDestination.print("UserKeyVal: ", pubgen.UserKeyVal[i], "\n");
                }
            }
            
            b64DecodeBuffer.reset();
            ByteCollector assemblyArea = b64DecodeBuffer.buffer;  

            bool ok = pubgen.outputPubOrSub(assemblyArea, &dest);

            if (!ok) {
                serialDestination.println("ERROR pubgen not ok");
                return ok;
            }
            return ok;
        }
    };

    // struct Request {

    //     slice nonc;
    //     slice message;
    //     slice pubk;
    //     // any more key/values?

    //     bool isHttp;
    //     bool isReply;
    //     bool isMqtt;
    //     bool isSerial;
    // };

    // struct connectionLater { // stream layer

    //     int poll(  Stream &s, sink theSink ){

    //         // get char if avail, add to buffer.
    //         // if whole packet
    //         //         send up to next layer
    //     }

    //     void push( Request & request){

    //         // write to stream

    //     }

    // };

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
