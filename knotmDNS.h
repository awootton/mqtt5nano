

#pragma once 

#if defined(ARDUINO)
#include <ESPmDNS.h>
#else

// This is a mock. The real one is in #include <ESPmDNS.h>
// example: 
// if (MDNS.begin("esp32"))
//   {
//     MDNS.setInstanceName("count server demo");
//     MDNS.addService("http", "tcp", 80);
//     MDNS.addServiceTxt("http","tcp","counter","true");
//   }

// I failed to implement mDns on my mac !!

struct mockMdns {

    bool begin( const  char * hostname){
        return true;
    }
    void setInstanceName(const char * name){

    }
    bool addService( const char * str1, const  char * str2, int port){
        return true;
    }
    bool addServiceTxt( const char * str1, const char * str2, const char * str3, const  char * str4){
        return true;
    }
};

extern mockMdns MDNS;

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
