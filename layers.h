

#pragma once

#include "slices.h"
#include "mockStream.h"

namespace mqtt5nano {

    struct Request {

        slice nonc;
        slice message;
        slice pubk;
        // any more key/values?

        bool isHttp;
        bool isReply;
        bool isMqtt;
        bool isSerial;
    };

    struct connectionLater { // stream layer

        
        int poll(  Stream &s, sink theSink ){

            // get char if avail, add to buffer.
            // if whole packet 
            //         send up to next layer
        }

        void push( Request & request){

            // write to stream

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
