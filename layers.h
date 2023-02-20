

#pragma once

#include "commandLine.h"
#include "eepromItem.h"
#include "httpConverter.h"
#include "mockStream.h"

#include "slices.h"

#include "commonCommands.h"
#include "nanoCommon.h"
#include "nanoCrypto.h"

// Pipeline is a series of routines to process a command.
// It is upside down from the pipeline in knotfree-net-homepage Pipeline.tsx

namespace mqtt5nano {

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
        char *b64DecodeBuffer = new char[BufferLen];

        SinkDrain response; // the total response

        SinkDrain commandOut; // just the output of the command.
        int commandOutLen = 0;

        SinkDrain b64DecodeSink;

        const char *errorString = nullptr;

        CommandPipeline() : response(cmdreplybuffer, BufferLen),
                            commandOut(cmdoutbuffer, BufferLen),
                            b64DecodeSink(b64DecodeBuffer, BufferLen) {
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
            if (b64DecodeBuffer != nullptr) {
                delete[] b64DecodeBuffer;
                b64DecodeBuffer = nullptr;
            }
        }

        slice b64Decode(slice s) {
            slice result = s.b64Decode(&b64DecodeSink.buffer);
            return result;
        }

        void handlePayload(slice payload, drain & socket) {

            SinkDrain& part1 = handlePayloadPart1( payload);

            if (wasHttp && ! isMqtt) {
                // make it into a http reply
                makeHttpReply(socket);
                //slice r ;//= response.buffer.slice();
               // socket.write(r) ;
            } 
            
            if (isMqtt) {
                // make it into a mqtt reply
                //slice r ;//= response.buffer.slice();
               // return commandOut;
            } else {
              // socket.write(commandOut.buffer.slice());
            }
            // if ( wasHttp){

            //     // make http results

            // } else {
            //     // if it was mqtt then make the pub and stuff it in the socket

            // }
            

        }

        SinkDrain& handlePayloadPart1(slice payload) {
            globalSerial->println("have payload");
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
                        errorString = "ERROR payload chop";
                        globalSerial->print(errorString);
                        globalSerial->println(chopped.error);
                        commandOut.print(chopped.error);
                        return commandOut;
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
                globalSerial->println(errorString);
                source = unknown;
            }

            // needs decryption if there's an admn key and a nonc key
            slice nonc = findParam("nonc");
            slice admn = findParam("admn");

            globalSerial->print("command is ");
            globalSerial->println(command->input.getCstr(b64DecodeSink.buffer.base + b64DecodeSink.buffer.start, b64DecodeSink.buffer.remaining()));

            bool isb64 = command->input.startsWith("=");
            if (isb64) {
                // decode into b64DecodeSink
                command->input.start++;

                // globalSerial->print("command is ");
                // globalSerial->println(command->input.getCstr(b64DecodeSink.buffer.base + b64DecodeSink.buffer.start, b64DecodeSink.buffer.remaining()));

                command->input = b64Decode(command->input);

                // globalSerial->print("command is ");
                // globalSerial->println(command->input.getCstr(b64DecodeSink.buffer.base + b64DecodeSink.buffer.start, b64DecodeSink.buffer.remaining()));
            }

            unsigned char adminpubkey[32];
            unsigned char pubkey[32];
            unsigned char privkey[32];
            unsigned char nonce[24];

            if (nonc.size() > 0 && admn.size() > 0) {
                // decrypt
                wasEncrypted = true;
                globalSerial->println("decrypting");

                slice adminPublicKey64 = adminPublicKeyStash.readSlice(b64DecodeSink);
                b64DecodeSink.buffer.start += adminPublicKey64.size();
                slice devicePrivateKey64 = devicePrivateKeyStash.readSlice(b64DecodeSink);
                b64DecodeSink.buffer.start += devicePrivateKey64.size();
                slice devicePublicKey64 = devicePublicKeyStash.readSlice(b64DecodeSink);
                b64DecodeSink.buffer.start += devicePrivateKey64.size();

                globalSerial->print("nonc is ");
                globalSerial->println(nonc.getCstr(b64DecodeSink.buffer.base + b64DecodeSink.buffer.start, b64DecodeSink.buffer.remaining()));

                globalSerial->print("admn is ");
                globalSerial->println(admn.getCstr(b64DecodeSink.buffer.base + b64DecodeSink.buffer.start, b64DecodeSink.buffer.remaining()));

                globalSerial->print("adminPublicKey64 is ");
                globalSerial->println(adminPublicKey64.getCstr(b64DecodeSink.buffer.base + b64DecodeSink.buffer.start, b64DecodeSink.buffer.remaining()));

                globalSerial->print("devicePrivateKey64 is ");
                globalSerial->println(devicePrivateKey64.getCstr(b64DecodeSink.buffer.base + b64DecodeSink.buffer.start, b64DecodeSink.buffer.remaining()));

                if (!adminPublicKey64.startsWith(admn)) {
                    errorString = "ERROR admn mismatch";
                    globalSerial->println(errorString);
                    // response.print(errorString);
                    // return response;
                }

                nonc.copy((char *)nonce, 24);
                sink dtmp = sink((char *)adminpubkey, 32);
                adminPublicKey64.b64Decode(&dtmp);
                sink dtmp2 = sink((char *)privkey, 32);
                devicePrivateKey64.b64Decode(&dtmp2);
                sink dtmp3 = sink((char *)pubkey, 32);
                devicePublicKey64.b64Decode(&dtmp2);

                int decryptedStart = b64DecodeSink.buffer.start;
                sink *dP = &(b64DecodeSink.buffer);
                bool ok = nanocrypto::unbox(dP, command->input, (char *)nonce, (char *)adminpubkey, (char *)privkey);
                if (!ok) {
                    errorString = "ERROR crypto::unbox";
                    globalSerial->println("ERROR crypto::unbox");
                    // response.print(errorString);
                    // return response;
                }
                command->input = slice(b64DecodeSink.buffer.base, decryptedStart, b64DecodeSink.buffer.start);

                globalSerial->print("command is "); // ie version#1676856950
                globalSerial->println(command->input.getCstr(b64DecodeSink.buffer.base + b64DecodeSink.buffer.start, b64DecodeSink.buffer.remaining()));

                int index = command->input.indexOf('#');
                if (index == -1) {
                    errorString = "ERROR no #";
                    globalSerial->println(errorString);
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
                    globalSerial->println(errorString);
                }

                { // re parse with badjson
                    // chop up the payload as text
                    badjson::ResultsTriplette chopped = badjson::Chop(newCommand.base + newCommand.start, newCommand.size());
                    if (chopped.error != nullptr) {
                        // return a message ??
                        errorString = "ERROR payload chop2";
                        globalSerial->print(errorString);
                        globalSerial->println(chopped.error);

                        delete chopped.segment;
                    }
                    if (command != nullptr) {
                        delete command;
                        command = nullptr;
                    }
                    command = chopped.segment;
                }
            }
            SinkDrain commandDrain = commandOut; // we output the command to the commandOut buffer
            if (wasEncrypted) {                  // unless it's going to me encrypted then we output to the response buffer instead and then encrypt it to commandOut
                commandDrain = response;
            }
            slice processedCommand = commandDrain.buffer;

            processedCommand.start = commandDrain.buffer.start;
            Command::process(command, params, commandDrain, source);
            if (wasEncrypted) {
                commandDrain.write("#");
                commandDrain.writeInt(getUnixTime());
            }
            processedCommand.end = commandDrain.buffer.start;

        bottom:

            // if it was encrypted then now it needs to be encrypted again.
            if (wasEncrypted) {
                slice ourCommandResult = processedCommand;
                if (errorString != nullptr) {
                    ourCommandResult = slice(errorString);
                }

                sink *dP = &(commandOut.buffer);
                bool ok = nanocrypto::box(dP, ourCommandResult, (char *)nonce, (char *)adminpubkey, (char *)privkey);
                if (!ok) {
                    errorString = "ERROR crypto::box";
                    globalSerial->println("ERROR crypto::box");
                    commandOut.print(errorString);
                }
            }

            // if (wasHttp) {
            //     // make it into a http reply
            //     makeHttpReply();
            //     return response;
            // } else {
            //     return commandOut;
            // }

            return commandOut;
        }

        void makeHttpReply( drain & destination) {

// fixme: make F work here
#if not defined(Arduino)
    #define F(a) a
#endif
            commandOutLen = commandOut.buffer.start;

            destination.print(F("HTTP/1.1 200 OK\r\n"));
            destination.print(F("Content-Length: "));

            destination.print(commandOutLen);
            destination.print(F("\r\n"));
            // we have to return the params as headers
            badjson::Segment *pP = params;
            while (pP != nullptr) {
                destination.write(pP->input);
                destination.print(": ");
                pP = pP->next;
                if (pP != nullptr) {
                    destination.write(pP->input);
                    pP = pP->next;
                }
                response.print(F("\r\n"));
            }
            if (isPng) { // hack alert
                destination.print(F("Content-Type: image/png\r\n"));
            } else {
                destination.print(F("Content-Type: text/plain\r\n"));
            }
            destination.print(F("Access-Control-Allow-Origin: *\r\n"));
            destination.print(F("Access-control-expose-headers: nonc\r\n"));
            destination.print(F("Connection: Closed\r\n"));
            destination.print(F("\r\n"));
            for (int i = 0; i < commandOutLen; i++) {
                destination.writeByte(commandOut.buffer.base[i]);
            }
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
