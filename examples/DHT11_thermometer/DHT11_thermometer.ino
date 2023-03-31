// DHT Temperature & Humidity Sensor
// Unified Sensor Library Example
// Written by Tony DiCola for Adafruit Industries
// Released under an MIT license.
// Depends on the following Arduino libraries:
// - Adafruit Unified Sensor Library: https://github.com/adafruit/Adafruit_Sensor
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library

// And example of mqtt5nano contolling a DHT11 sensor which can be read remotely by the app at knotfree.net
// depends on this library: mqtt5nano

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN D7 // Pin which is connected to the DHT sensor.

#define DHTTYPE DHT11 // DHT 11

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

// Now let's add the KnotFree/mqtt5nano stuff
// we'll have two commands. One for temp and one for humidity.

#include "mqtt5nano.h"

mqtt5nano::PackageOne one; // instantiate the library.

struct getTemperatureF : Command {
    void init() override {
        name = "get f";
        description = "temperature in °F 🔓";
    }
    void execute(Args args, badjson::Segment *params, Destination &out) override {
        sensors_event_t event;
        dht.temperature().getEvent(&event);
        float f = event.temperature * 9;
        f = f / 5;
        f = f + 32;
        out.writeFloat((int)f, 2); // two digits after the decimal point.
        out.write("°F");
    }
};
getTemperatureF cmdf; // instantiate the command.

struct getTemperatureC : Command {
    void init() override {
        name = "get c";
        description = "temperature in °C 🔓";
    }
    void execute(Args args, badjson::Segment *params, Destination &out) override {
        sensors_event_t event;
        dht.temperature().getEvent(&event);
        float f = event.temperature;
        out.writeFloat((int)f, 2); // two digits after the decimal point.
        out.write("°C");
    }
};
getTemperatureC cmdc;// instantiate the command.


struct getHumidity : Command {
    void init() override {
        name = "get humidity";
        description = "humidity in % 🔓";
    }
    void execute(Args args, badjson::Segment *params, Destination &out) override {

        sensors_event_t event;
        dht.temperature().getEvent(&event);
        float f = event.relative_humidity;
        out.writeInt((int)f);  
        out.write("%");
    }
};
getHumidity cmdh;// instantiate the command.

void setup() {
    Serial.begin(115200);
    delay(delayMS);

    // Initialize device.xs
    dht.begin();

    Serial.println("DHTxx Unified Sensor Example");
    // Print temperature sensor details.
    sensor_t sensor;
    dht.temperature().getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.println("Temperature");
    Serial.print("Sensor:       ");
    Serial.println(sensor.name);
    Serial.print("Driver Ver:   ");
    Serial.println(sensor.version);
    Serial.print("Unique ID:    ");
    Serial.println(sensor.sensor_id);
    Serial.print("Max Value:    ");
    Serial.print(sensor.max_value);
    Serial.println(" *C");
    Serial.print("Min Value:    ");
    Serial.print(sensor.min_value);
    Serial.println(" *C");
    Serial.print("Resolution:   ");
    Serial.print(sensor.resolution);
    Serial.println(" *C");
    Serial.println("------------------------------------");
    // Print humidity sensor details.
    dht.humidity().getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.println("Humidity");
    Serial.print("Sensor:       ");
    Serial.println(sensor.name);
    Serial.print("Driver Ver:   ");
    Serial.println(sensor.version);
    Serial.print("Unique ID:    ");
    Serial.println(sensor.sensor_id);
    Serial.print("Max Value:    ");
    Serial.print(sensor.max_value);
    Serial.println("%");
    Serial.print("Min Value:    ");
    Serial.print(sensor.min_value);
    Serial.println("%");
    Serial.print("Resolution:   ");
    Serial.print(sensor.resolution);
    Serial.println("%");
    Serial.println("------------------------------------");

    one.setup(Serial);
}

void loop() {
    one.loop(millis(), Serial);
}
