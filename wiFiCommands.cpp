

#include "wiFiCommands.h"
#include "commonCommands.h"

namespace mqtt5nano {

    bool connected = false;

    int conectCountdown = 0;

    bool did_mDns = false;

    ssidGet ssidGetCommand;

    ssidSet ssidSetCommand;

    passGet passGetCommand;

    passSet passSetCommand;

    wifiStatus wifiStatusCmd;

    EepromItem ssidStash(32, "ssid","");

    EepromItem passStash(48, "wifi pass","");

    EepromItem hostStash(32, "short name","");

    hostGet hostGetCommand;

    hostSet hostSetCommand;

    favIcon favIconCmd;

    ssidListGet listWifiCmd;

    peersListGet peersListGetCmd;

    freeMem freeMemCmd;


    void writeStarredPass( EepromItem &stash, Destination &out) {
        char buff[stash.size];
        ByteDestination tmp(buff, stash.size);
        stash.read(tmp);
        if ( tmp.getWritten().size() == 0 ){
            return;
        }
        for (int i = 1; i < 8; i++) {
           out.writeByte('*');
        }
    }

} // namespace

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
