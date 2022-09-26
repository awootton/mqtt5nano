
#pragma once

#if defined(ARDUINO)
//  Nothing. Serial is always defined.
#else

// This is a mock.
// because there's no Serial when not in arduino !!!!
// so this file keeps the compiler happy when testng with C++
// do we need functinality here?
// the wifi tcp uses this same interface.
// TODO: hook it up to stdio

#include <iostream>
//#include <string>
#include <stdio.h>
#include <string.h>

using namespace std;

class Stream
{
public:

// todo: make the output a drain instead of always cout

    //    size_t print(const String & str){
    //     return 0;
    //    }
    virtual size_t print(const char *dp)
    {
        cout << dp;
        return 1;
    }
    virtual size_t println()
    {
        cout << "\n";
        return 1;
    }
    virtual size_t println(const char *dp)
    {
        cout << dp << "\n";
        return 1;
    }
    virtual size_t println(int i)
    {
        cout << i << "\n";
        return 0;
    }
    size_t print(char c)
    {
        cout << c;
        return 1;
    }
    virtual int available()
    {
        return 0;
    }
    virtual char read()
    {
        return 'a';
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
