

#pragma once

#include "slices.h"

namespace mqtt5nano {

    struct WizardScreen {

        ~WizardScreen() {}

        virtual void makeMessage(Destination &out){};

        virtual void execute(int picked, const char *typed){};
    };

    WizardScreen *makeGetWifi();
    WizardScreen *makeGetPass();
    WizardScreen *makeGetShortName();
    void pushWizard(WizardScreen *a);
    void makeMessage(Destination &out);
    void execute(int picked, const char *typed);
    void clearScreen(Destination &out);
    int wizardStackSize();

    struct intro : WizardScreen {

        ~intro() {
            // delete ??
        }

        void makeMessage(Destination &out) override {

            clearScreen(out);

            out.write("Welcome to the wifi set up wizard.\n");
            out.write("Type one of the numbers from the list below. Press return.\n");
            out.write("1) Continue to set wifi ssid.\n");
            out.write("9) Quit. I'll do it manually.\n");
            out.write("0) help.\n");
        }
        void execute(int picked, const char *typed) override {

            if (picked == 1) {

                WizardScreen *next = makeGetWifi();
                pushWizard(next);

            } else if (picked == 2) {

            } else if (picked == 3) {

            } else {
            }
        }
    };

    struct getWifi : WizardScreen {

        ~getWifi() {
            // delete the ssid list
        }

        void makeMessage(Destination &out) override {

            clearScreen(out);

            out.write("Type the number of the wifi that you are using. or 0.\n");
            out.write("0) Manually enter ssid name.\n");
        }
        void execute(int picked, const char *typed) override {

            // cout << "getWifi" << picked << typed << "/n";

            if (picked == 2) {
                // WizardScreen * next = new
            }
        }
    };

    struct getPass : WizardScreen {

        ~getPass() {
            // delete the ssid list
        }

        void makeMessage(Destination &out) override {

            clearScreen(out);

            out.write("Type the number of the wifi that you are using. or 0.\n");
            out.write("0) Manually enter ssid name.\n");
            out.write("0) Manually enter ssid name.\n");
            out.write("0) Manually enter ssid name.\n");
        }
        void execute(int picked, const char *typed) override {

            // cout << "getWifi" << picked << typed << "/n";

            if (picked == 2) {
                // WizardScreen * next = new
            }
        }
    };

    struct getShortName : WizardScreen {

        ~getShortName() {
            // delete the ssid list
        }

        void makeMessage(Destination &out) override {

            clearScreen(out);

            out.write("Type the number of the wifi that you are using. or 0.\n");
            out.write("0) Manually enter short name.\n");
        }
        void execute(int picked, const char *typed) override {

            // cout << "getWifi" << picked << typed << "/n";

            if (picked == 2) {
                // WizardScreen * next = new
            }
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
