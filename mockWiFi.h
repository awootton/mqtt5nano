

#if defined(ARDUINO)
#include <WiFi.h>
#else

// else we mock it.
#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "mockStream.h"

#include "slices.h"

using namespace std;

#pragma once

typedef enum
{                       // from ESP8266WiFiType.h
    WIFI_MODE_NULL = 0, /**< null mode */
    WIFI_MODE_STA,      /**< WiFi station mode */
    WIFI_MODE_AP,       /**< WiFi soft-AP mode */
    WIFI_MODE_APSTA,    /**< WiFi station + soft-AP mode */
    WIFI_MODE_MAX
} wifi_mode_t;

typedef enum
{
    WL_NO_SHIELD = 255, // for compatibility with WiFi Shield library
    WL_IDLE_STATUS = 0,
    WL_NO_SSID_AVAIL = 1,
    WL_SCAN_COMPLETED = 2,
    WL_CONNECTED = 3,
    WL_CONNECT_FAILED = 4,
    WL_CONNECTION_LOST = 5,
    WL_DISCONNECTED = 6
} wl_status_t;

#define WiFiMode_t wifi_mode_t
#define WIFI_OFF WIFI_MODE_NULL
#define WIFI_STA WIFI_MODE_STA
#define WIFI_AP WIFI_MODE_AP
#define WIFI_AP_STA WIFI_MODE_APSTA

struct IPAddress
{
    unsigned char parts[4];
    IPAddress(int a, int b, int c, int d)
    {
        parts[0] = a;
        parts[1] = b;
        parts[2] = c;
        parts[3] = d;
    }
    uint8_t operator[](int index) const
    {
        return parts[index];
    }
};

struct mockWiFi
{

    // eg.
    //   WiFi.mode(WIFI_STA);
    //   WiFi.begin("woot2", "fly1440nap");
    //   while (WiFi.status() != WL_CONNECTED)
    //   WiFi.localIP()

    const char *saved_ssid;
    const char *saved_passphrase;

    bool mode(wifi_mode_t m)
    {
        return true;
    }

    wl_status_t begin(const char *ssid, const char *passphrase)
    {
        saved_ssid = ssid;
        saved_passphrase = passphrase;
        return status();
    }

    wl_status_t status()
    {
        if (strlen(saved_ssid) == 0)
        {
            return WL_NO_SSID_AVAIL;
        }
        if (strlen(saved_passphrase) == 0)
        {
            return WL_NO_SSID_AVAIL;
        }
        return WL_CONNECTED;
    }

    const IPAddress localIP()
    {
        return IPAddress(1, 2, 3, 4);
    }
};

extern mockWiFi WiFi;

struct CoutDrain2 : mqtt5nano::drain // for the examples output to cout
{
    bool writeByte(char c) override
    {
        std::cout << c;
        bool ok = true;
        return ok;
    };
};
// CoutDrain2 my_cout_drain;

// TODO: we could actually make this work instead of being a stub.
struct mockWiFiClientBuffers : Stream
{
    //  eg.  if (!client.connect(host, port))
    // client.print("GET / HTTP/1.1\r\nHost: ");
    // client.available()
    // client.readStringUntil('\r') not used
    // client.stop();

    int client_fd;
    char got;
    bool have = false;

    CoutDrain2  output;

    char buff[16000];
    mqtt5nano::slice  source;//(buff,0,0);


    bool connected()
    {
        return client_fd != -1; // Let's remember this doesn't actually work.
    }

    // can't call this connect becaause it conflicts with connect
    int xconnect(const char *host, uint16_t port)
    {  
        client_fd = 12345;

        return 11;
    }

    char read()
    {
        char c = source.base[source.start++];
        return c;
    }

    void print(char c)
    {
        output.writeByte(c);
    }

    void write(const char *cP)
    {
        output.write(cP);
    }
    // int print(const char *cP)
    // {
    //     int slen = send(sock, &cP, strlen(cP), 0);
    // }

    void setNoDelay(bool d)
    {
    }
    void flush()
    {
    }

    void fillChar()
    {
    }

    int available()
    {
       if ( source.empty() ){
         return 0;
       }
       return 1;
    }
    bool stop()
    {
        // if (client_fd != -1)
        // {
        //     close(client_fd);
        // }
        client_fd = -1;
        return true;
    }
};

struct mockWiFiClientTcp : Stream
{
    //  eg.  if (!client.connect(host, port))
    // client.print("GET / HTTP/1.1\r\nHost: ");
    // client.available()
    // client.readStringUntil('\r') not used
    // client.stop();

    int sock;
    int client_fd;
    char got;
    bool have = false;

    bool connected()
    {
        return client_fd != -1; // Let's remember this doesn't actually work.
    }

    // can't call this connect becaause it conflicts with connect
    int xconnect(const char *host, uint16_t port)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            return -1;
        }
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Socket creation error \n");
            sock = 0;
            return -1;
        }
        struct sockaddr_in serv_addr;

        serv_addr.sin_family = AF_INET;
        // int port = 80;
        serv_addr.sin_port = htons(port);

        int ok = inet_pton(AF_INET, host, &serv_addr.sin_addr);

        client_fd = connect(sock, (struct sockaddr *)&serv_addr,
                            sizeof(serv_addr));

        return sock;
    }

    char read()
    {
        char result = got;
        have = false;
        fillChar();
        return result;
    }

    void print(char c)
    {
        int slen = send(sock, &c, 1, 0);
    }

    void write(const char *cP)
    {
        int slen = send(sock, &cP, strlen(cP), 0);
    }
    // int print(const char *cP)
    // {
    //     int slen = send(sock, &cP, strlen(cP), 0);
    // }

    void setNoDelay(bool d)
    {
    }
    void flush()
    {
    }

    void fillChar()
    {
        if (have)
        {
            return;
        }
        char c = 0;
        int n = recv(sock, &c, 1, MSG_DONTWAIT); // async !!!
        // n == 1 means we got some
        // n == 0 mean socket was closed
        // n == -1 means no data received

        if (n == 1)
        {
            have = true;
            got = c;
        }
        else if (n == 0)
        {
            stop(); // we disconnected
        }
        else if (n == -1)
        {
            // nothing.
        }
        else
        {
            // idk
        }
    }

    int available()
    {
        if ((client_fd == -1) || (sock = -1))
        {
            return 0;
        }
        fillChar();
        if (have)
        {
            return 1;
        }
        return 0;
    }
    bool stop()
    {
        if (client_fd != -1)
        {
            close(client_fd);
        }
        client_fd = -1;
        return true;
    }
};

// Because we can't call inet connect inside a method named connect!
struct WiFiClient : mockWiFiClientBuffers
{
    int connect(const char *host, uint16_t port)
    {
        return xconnect(host, port);
    }
    bool operator==(const WiFiClient &rhs)
    {
        return client_fd == rhs.client_fd; // not how it really works
    }
};

// extern mockWiFiClient2 WiFiClient;

struct WiFiServer
{

    WiFiClient client1;
    WiFiClient client2;

    WiFiServer(int port)
    {
    }

    void begin()
    {
    }
    // WiFiClient client = server.available();
    WiFiClient available()
    {
        return client1;
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
