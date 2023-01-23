
#include "stdint.h"

#include <cstdlib>
#include <iostream>

#include <fstream>

#include <random>

#include "slices.h"

#include "mockmDNS.h"
#include "nanobase64.h"
#include "setupWizard.h"

using namespace std;
using namespace mqtt5nano;

struct getWifi;

struct CoutDrain : drain // for the examples output to cout
{
    bool writeByte(char c) override {
        cout << c;
        bool ok = true;
        return ok;
    };
};
CoutDrain out;

// struct WizardScreen {

//     ~WizardScreen() {}

//     virtual void makeMessage(){};

//     virtual void execute(int picked, const char *typed){};
// };

// WizardScreen *makeGetWifi();
// void push(WizardScreen *a);

// void clearScreen() {
//     for (int i = 0; i < 10; i++) {

//         out.write(" \n");
//         out.write("\n");
//     }
// }

// struct intro : WizardScreen {

//     ~intro() {
//         // delete ??
//     }

//     void makeMessage() override {

//         clearScreen();

//         out.write("Welcome to the wifi set up wizard.\n");
//         out.write("Type  one of the numbers from the list below.\n");
//         out.write("1) Continue to set wifi ssid.\n");
//         out.write("9) Quit. I'll do it manually.\n");
//         out.write("0) help.\n");
//     }
//     void execute(int picked, const char *typed) override {

//         if (picked == 1) {

//             WizardScreen *next = makeGetWifi();
//             push(next);

//         } else if (picked == 2) {

//         } else if (picked == 3) {

//         } else {
//         }
//     }
// };

// struct getWifi : WizardScreen {

//     ~getWifi() {
//         // delete the ssid list
//     }

//     void makeMessage() override {

//         clearScreen();

//         out.write("Type the number of the wifi that you are using. or 0.\n");
//         out.write("0) Manually enter ssid name.\n");
//     }
//     void execute(int picked, const char *typed) override {

//         cout << "getWifi" << picked << typed << "/n";

//         if (picked == 2) {
//             // WizardScreen * next = new
//         }
//     }
// };

// struct getPass : WizardScreen {

//     ~getPass() {
//         // delete the ssid list
//     }

//     void makeMessage() override {

//         clearScreen();

//         out.write("Type the number of the wifi that you are using. or 0.\n");
//         out.write("0) Manually enter ssid name.\n");
//     }
//     void execute(int picked, const char *typed) override {

//         cout << "getWifi" << picked << typed << "/n";

//         if (picked == 2) {
//             // WizardScreen * next = new
//         }
//     }
// };

// struct getShortName : WizardScreen {

//     ~getShortName() {
//         // delete the ssid list
//     }

//     void makeMessage() override {

//         clearScreen();

//         out.write("Type the number of the wifi that you are using. or 0.\n");
//         out.write("0) Manually enter ssid name.\n");
//     }
//     void execute(int picked, const char *typed) override {

//         cout << "getWifi" << picked << typed << "/n";

//         if (picked == 2) {
//             // WizardScreen * next = new
//         }
//     }
// };

// WizardScreen *makeGetWifi() {
//     return new getWifi();
// }

// WizardScreen *makeGetPass() {
//     return new getPass();
// }

// WizardScreen *makeGetShortName() {
//     return new getShortName();
// }

// WizardScreen *current[8];
// int currentLevel = 0;

// void push(WizardScreen *a) {
//     current[currentLevel++] = a;
//     if (currentLevel == 8) {
//         int s = (char *)(current + 1) - (char *)(current + 8);
//         memmove(current + 0, current + 1, s);
//         currentLevel--;
//     }
// }

bool running = true;
void loop() {
    drain *dest = &out;

    makeMessage(*dest);

    std::string str;
    std::getline(std::cin, str);

    cout << "got " << str << "\n";
    int picked = 0;
    picked = std::stoi(str);
    cout << "picked " << picked << "\n";

    execute(picked, str.c_str());
}

int main(void) {

       // 112 bytes
    const char *png64 = "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAGklEQVR42mNUatH/z0ABYBw1YNSAUQOGiwEA76odUYu8G30AAAAASUVORK5CYII";
    char png[256];
    int len = base64::decode((const unsigned char *)png64, strlen((const char *)(png64)), png, 256);
    cout << len << " len \n";
    cout << "{ ";
    for (int i = 0; i < len; i++) {
        if ((i % 16) == 15) {
            cout << "\n";
        }
        uint8_t c = png[i];
        cout << (int)c << ",";
    }
    cout << "} \n";

    char dec[256];
    len = base64::encode((const unsigned char *)png, len, dec, 256);
    dec[len] = 0;

    cout << dec << "\n";

    static unsigned char arr[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,
                                82,0,0,0,16,0,0,0,16,8,6,0,0,0,31,243,
                                255,97,0,0,0,26,73,68,65,84,120,218,99,84,106,209,
                                255,207,64,1,96,28,53,96,212,128,81,3,134,139,1,0,
                                239,170,29,81,139,188,27,125,0,0,0,0,73,69,78,68,
                                174,66,96,130,} ;

    // cpp write file
    // Create and open a text file
    fstream my_file;
    my_file.open("favicon.png", ios::out);
    if (!my_file) {
        cout << "File not created!";
    } else {
        cout << "File created successfully!";
        for (int i = 0; i < len; i++) {
           // my_file.put(png[i]);
            my_file.put(arr[i]);
        }
        my_file.close();
    }

    pushWizard(new intro());

    while (running) {
        loop();
    }
}
