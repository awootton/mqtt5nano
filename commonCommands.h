
#pragma once

#include "commandLine.h"
#include "eepromItem.h"

#include "nanoCrypto.h"
#include "nanobase64.h"

namespace mqtt5nano {

    extern EepromItem adminPublicKeyStash;
    extern EepromItem devicePublicKeyStash;
    extern EepromItem devicePrivateKeyStash;
    void getTokenPayload(Destination &out);

    struct adminKeyGet : Command {
        void init() override {
            name = "get admin hint";
            description = "admin key start.🔓";
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {
            int prev = adminPublicKeyStash.size;
            adminPublicKeyStash.size = 8;
            adminPublicKeyStash.read(out);
            adminPublicKeyStash.size = prev;
        }
    };

    struct pubKeyGet : Command {
        void init() override {
            name = "get pubk";
            description = "device public key.🔓";
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {
            devicePublicKeyStash.read(out);
        }
    };

    void setAdminPassword(slice arg);

    struct adminKeySet : Command {
        void init() override {
            name = "set admin password";
            description = "the passphrase for admin access.";
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {

            if (args.source != CommandSource::SerialPort) {
                out.write("ERROR admin key can only be set from a Serial Monitor");
                return;
            }
            if (args[0].empty()) {
                out.write("ERROR expected a value");
                return;
            }

            setAdminPassword(args[0]);

            out.write("OK");
        }
    };


    
    //  {
    //     char adminPublicKey[32];
    //     char adminPrivateKey[32];
    //     char tmp[1024];
    //     char *passphrase = arg.getCstr(tmp, 1024);

    //     nanocrypto::getBoxKeyPairFromPassphrase(passphrase, adminPublicKey, adminPrivateKey);
    //     int len = base64::encode(adminPublicKey, 32, tmp, 1014);
    //     adminPublicKeyStash.write(slice(tmp, 0, len));
    // }

    void setDevicePassword(slice arg);

    struct deviceKeySet : Command {
        void init() override {
            name = "set device password";
            description = "the passphrase for this device.";
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {

            if (args.source != CommandSource::SerialPort) {
                out.write("ERROR device keys can only be set from a Serial Monitor");
                return;
            }
            if (args[0].empty()) {
                out.write("ERROR expected a value");
                return;
            }
            setDevicePassword(args[0]);
            out.write("OK");
        }
    };


    extern EepromItem hostStash;
    extern bool did_mDns;

    struct hostSet : Command {
        void init() override {
            name = "set short name";
            description = "the '.local.' name.";
            argumentCount = 1;
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {
            if (args[0].empty()) {
                out.write("ERROR expected a value");
                return;
            }
            hostStash.write(args[0]);
            out.write("ok: ");
            hostStash.read(out);
            did_mDns = false; // retry now
        }
    };

    void setShortName( const char *name );
    void setLongName(const char * name);

    struct getUptime : Command {

        int starttime = 0;
        void init() override {
            name = "uptime";
            description = "time since last reboot.";
            starttime = latestNowMillis;
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {
            long delta = latestNowMillis - starttime;
            delta = delta / 1000;
            int seconds = delta % 60;
            delta = delta / 60;
            int minutes = delta % 60;
            delta = delta / 60;
            int hours = delta;
            out.writeInt(hours);
            out.write(" hours ");
            out.writeInt(minutes);
            out.writeByte(':');
            out.writeInt(seconds);
        }
    };

    struct getServed : Command {
        void init() override {
            name = "served";
            description = "count of requests served since reboot.";
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {
            out.writeInt(commandsServed);
        }
    };

    struct getVersion : Command {
        void init() override {
            name = "version";
            description = "mqtt5nano version";
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {
            out.write("v0.3.0 mqtt5nano");
        }
    };

    struct getUnixTimeCmd : Command {
        void init() override {
            name = "get time";
            description = "seconds since 1970🔓";
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {
            out.writeInt(getUnixTime());
        }
    };

    struct setUnixTimeCmd : Command {
        void init() override {
            name = "set time";
            description = "adjust the clock as necessary. Not safe.";
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {
            if (args[0].empty()) {
                out.write("ERROR expected a value");
                return;
            }
            setUnixTime(args[0].toLong());
        }
    };


    extern EepromItem topicStash;
    extern EepromItem tokenStash;

    struct mqttStatus : Command {

        void init() override {
            name = "status";
            description = "mqtt status";
        }
        void execute(Args args, badjson::Segment *params, Destination &out) override {
            out.write("admin hint:");
            int prev = adminPublicKeyStash.size;
            adminPublicKeyStash.size = 8;
            adminPublicKeyStash.read(out);
            adminPublicKeyStash.size = prev;

            out.write(" admin public key:");
            adminPublicKeyStash.read(out);

            out.write(" public key:");
            devicePublicKeyStash.read(out);

            out.write(" long name:");
            topicStash.read(out);

            out.write(" token:");
            //xx xx  tokenStash.read(out);
            getTokenPayload(out);
            out.write("\n");
        };
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
