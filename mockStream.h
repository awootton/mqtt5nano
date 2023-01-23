
#pragma once

#if defined(ARDUINO)
//  Nothing. Serial is always defined.
#include <Arduino.h>
#else

extern void delay(int);

// This is a mock.
// because there's no Serial when not in arduino !!!!
// so this file keeps the compiler happy when testng with C++
// do we need functinality here?
// the wifi tcp uses this same interface.
// TODO: hook it up to stdio

#include <iostream>
#include <stdio.h>
#include <string.h>

#include "slices.h"

using namespace std;
using namespace mqtt5nano;

class Stream {
public:
    // todo: make the output a drain instead of always cout

    //    size_t print(const String & str){
    //     return 0;
    //    }
    virtual size_t print(const char *dp) {
        cout << dp;
        return 1;
    }
    virtual size_t println() {
        cout << "\n";
        return 1;
    }
    virtual size_t println(const char *dp) {
        cout << dp << "\n";
        return 1;
    }
    virtual size_t println(int i) {
        cout << i << "\n";
        return 0;
    }
    virtual size_t print(char c) {
        cout << c;
        return 1;
    }
    virtual int available() {
        return 0;
    }
    virtual char read() {
        return 'a';
    }
};


struct DrainStream : Stream {

    int client_fd;
    char got;
    bool have = false;

    char outputbuffer[16000];
    SinkDrain outdrain; // ((char *)outputbuffer,sizeof(outputbuffer));

    mqtt5nano::drain *output = &outdrain;//&my_cout_drain;

    char buff[16000];
    mqtt5nano::slice source; //(buff,0,0);

    DrainStream(){
            source.base = buff;
            source.start = 0;
            source.end = sizeof(buff);
    }

    bool connected() {
        return client_fd != -1; // Let's remember this doesn't actually work.
    }

    // can't call this connect becaause it conflicts with connect
    int xconnect(const char *host, uint16_t port) {
        client_fd = 12345;

        return 11;
    }

    char read() override {
        char c = source.readByte();//source.base[source.start++];
        return c;
    }

    size_t print(char c) override {
        bool ok = output->writeByte(c);
        return ok?1:0;
    }

    void write(  uint8_t c) {
        output->writeByte(c);
    }
    void write(char c) {
        output->writeByte(c);
    }

    void write(const char *cP) {
        output->write(cP);
    }
    size_t print(const char *dp) override {
        mqtt5nano::slice s(dp);
        output->write(s);
        return s.size();
    }
    size_t println() override {
        output->writeByte('\n');
        return 1;
    }
    size_t println(const char *dp) override {
        mqtt5nano::slice s(dp);
        output->write(s);
        output->writeByte('\n');
        return s.size() + 1;
    }

    int available()  override {
        return source.size();
    }

    void setNoDelay(bool d) {
    }
    void flush() {
    }

    void fillChar() {
    }

    // int available() {
    //     if (source.empty()) {
    //         return 0;
    //     }
    //     return 1;
    // }
    bool stop() {
        // if (client_fd != -1)
        // {
        //     close(client_fd);
        // }
        client_fd = -1;
        return true;
    }
};

#endif

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
