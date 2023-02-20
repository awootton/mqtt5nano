

#include "eepromItem.h"
#include "commandLine.h"
#include "nanoCommon.h"

namespace mqtt5nano {
    EepromItem *eehead;

    int totalSize = 0;

    EepromItem::EepromItem(int size, const char *description, const char *initialValue)
        : size(size), description(description), initialValue(initialValue) {
        offset = totalSize;
        totalSize += size;
        EepromItem *nxt = eehead;
        eehead = this;
        this->next = nxt;
    }

    EepromItem *getEitemHead() {
        return eehead;
    }
    int getEitemTotal() {
        return totalSize;
    }

    void initAllEeItem() {
        globalSerial->println("# Ee Init");
        EepromItem *eP = eehead;
        while (eP != nullptr) {
            // s.print("ee checking ");
            // s.print(eP->description);
            // s.print(" ");
            // s.print(eP->offset);
            // s.print(" ");
            // s.print(eP->size);
            // s.print("\n");
            eP->checkStarted();
            eP = eP->next;
        }
    }
    // erase command
    struct eraseAll : Command {
        void init() override {
            name = "settings erase";
            description = "erase all the settings with code KILLMENOW";
            this->argumentCount = 1;

            // serial only
        }
        void execute(Args args, badjson::Segment *params, drain &out) override {
                if ( args.count() == 0 ){
                    out.write("arg expected");
                    return;
                }
                if ( ! args[0].equals("KILLMENOW") ){
                    out.write("wrong arg");
                    return;
                }
                for ( int i = 0; i < totalSize; i ++ ){
                    EEPROM.write(i,0xFF);
                }
                EEPROM.commit();
                out.write("ok. You should reboot now.");
        }
    };

    eraseAll ea;

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
