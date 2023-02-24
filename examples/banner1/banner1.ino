
#include "mqtt5nano.h"

#include <MD_MAX72xx.h>
#include <MD_Parola.h>
#include <SPI.h>

// Arduino code for MAX7219 as an IOT things using mqtt5nano and Parola.

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW // or maybe PAROLA_HW
#define MAX_DEVICES 4

#ifdef ESP8266
#define CLK_PIN D5  // D5 aka sck aka gpio 14 aka HSCK
#define DATA_PIN D7 // D7 aka mosi aka gpio 13 aka RXD2 aka HMOSI
#define CS_PIN D8       // D8 aka cs aka gpio 15 aka TXD2 aka HCS
#endif

#ifdef ESP32
#define CLK_PIN 18  //GPIO18
#define DATA_PIN 23 //GPIO23
#define CS_PIN 5    // GPIO5
#endif

// HARDWARE SPI:
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// SOFTWARE SPI:
// MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// TODO: add commands to change these.
uint8_t scrollSpeed = 25; // default frame delay value
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
uint16_t scrollPause = 100; // 2000; // in milliseconds

// Global message buffers shared by Serial and Scrolling functions
#define BUF_SIZE 1024
char curMessage[BUF_SIZE] = {"            ."};

// end of the Parola code

// start of the mqtt5nano code.

mqtt5nano::PackageOne one; // pull in all the utils

EepromItem bannerStash(1024,"banner",",        .       "); // create eeprom storage

// define a comand
struct setBanner : Command {
  
    void init() override {
        //static const char PROGMEM tmp[] = "set banner";
        name = "set banner";
        //static const char PROGMEM tmp2[] = "change the banner";
        description = "change the banner";
        argumentCount = 1;
    }
    void execute(Args args, badjson::Segment *params, Destination &out) override {

        ByteDestination msg(curMessage, BUF_SIZE);

        if (args[0].empty()) {
            // static const char PROG MEM tmp[] = "ERROR expected a value";
            out.write("ERROR expected a value");
            return;
        }

        msg.write(args[0]);
        msg.writeByte(0);

        bannerStash.write(args[0]);

        out.write("ok");

        P.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
    }
};

struct getBanner : Command {
    void init() override {
       // static const char PROGMEM tmp[] = "get banner";
        name = "get banner";
        // static const char PROGMEM tmp2[] = " the banner text";
        description = " the banner text";
    }
    void execute(Args args, badjson::Segment *params, Destination &out) override {
        out.write(curMessage);
    }
};

// important, you have to do this to make them exist. 
setBanner sb;
getBanner gb;

void setup() {
    Serial.begin(115200);
    delay(100);

    one.setup(Serial);

    // init the Parola buffer from the eeprom.
    ByteDestination tmp(curMessage, sizeof(curMessage));
    bannerStash.read(tmp);

    P.begin();
    P.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);

    Serial.println("setup done");
}

void loop() {

    one.loop(millis(), Serial);

    if (P.displayAnimate()) {
        P.displayReset();
    }
}
