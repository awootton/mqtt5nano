

#include <iostream>
//#include <vector>
//#include <stdio.h>
#include <string>
//#include <string.h> // has strcmp

#include <stdlib.h>

/* itoa example */
#include <stdio.h>
#include <stdlib.h>

#include "commandLine.h"

#include "commonCommands.h"

#include "streamReader.h"

#include "wiFiCommands.h"

#include "httpServices.h"

#include "httpConverter.h"

#include "badjson.h"

#include "mqttCommands.h"

#include "knotStream.h"

#include "knotbase64.h"



using namespace std;
using namespace badjson;



int main()
{
    cout << "hello mqtt commands tests\n";

    MqttCommandClient client;

    Stream  serial;

    // some binary for mqtt publish
    const char *pubbytes = "3268000d544553542f54494d456162636400032d080010544553542f54494d4565666768696a6b2600046b657931000476616c312600046b657932000476616c326d65737361676520617420323032302d30332d32372030313a33353a33372e34303330373920633d31";
    char tmpbuffer[1024];
    int amt = hex::decode((const unsigned char*)pubbytes, strlen(pubbytes), tmpbuffer, 1024);

    client.client.source = slice(tmpbuffer,0,amt);

    for ( int i = 0 ; i < 1000; i ++ ) {

        client.loop(i, serial);

        connected = true;
    }
   
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

