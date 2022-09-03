

// Copyright 2020 Alan Tracey Wootton
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

#include <iostream>
#include <vector>
#include <string>

#include "mqtt5nano.h"

#include <string.h> // has strcmp

#include <arpa/inet.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

bool sendSubscribe( int  sock );

unsigned int millis();

using namespace std;

using namespace knotfree;

// it's a bug that this token still works. 
const char *password = "[Free_token_expires:_2021-12-31,{exp:1641023999,iss:_9sh,jti:HpifIJkhgnTOGc3EDmOJaV0A,in:32,out:32,su:4,co:2,url:knotfree.net},eyJhbGciOiJFZDI1NTE5IiwidHlwIjoiSldUIn0.eyJleHAiOjE2NDEwMjM5OTksImlzcyI6Il85c2giLCJqdGkiOiJIcGlmSUpraGduVE9HYzNFRG1PSmFWMEEiLCJpbiI6MzIsIm91dCI6MzIsInN1Ijo0LCJjbyI6MiwidXJsIjoia25vdGZyZWUubmV0In0.YSo2Ur7lbkwTPZfQymyvy4N1mWQaUn_cziwK36kTKlASgqOReHQ4FAocVvgq7ogbPWB1hD4hNoJtCg2WWq-BCg]";

slice onePacketSimpleExample(mqttPacketPieces &parser, const slice availableNow);

int main()
{

    cout << "Hello World! test tcp async mqtt \n";

    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    struct hostent *host = gethostbyname("knotfree.net");
    if (!host)
    {
        printf("unable to resolve\n");
        return -1;
    }

    char * ipaddr  = 0;
    if (host->h_addr_list[0] != nullptr)
    {
        ipaddr = inet_ntoa((struct in_addr) * ((struct in_addr *)host->h_addr_list[0]));
    }
    if ( ! ipaddr ){
        cout << "no ip addr found \n";
        return -1;
    }

    //  struct
    //    addrinfo hints;
    // bzero((void *)&hints, sizeof(hints));
    // hints.ai_family = AF_UNSPEC;
    // hints.ai_socktype = SOCK_STREAM;
    // hints.ai_protocol = IPPROTO_TCP;

    // char service[6];
    // bzero(service, sizeof(service));
    // sprintf(service, "%hu", port_number);

    // struct addrinfo *addrs = 0;
    // if (getaddrinfo(hostname, service, &hints, &addrs) != 0)
    // {
    //     printf("unable to resolve\n");
    //     return false;
    // }

    // for(struct addrinfo *addr = addrs; addr != 0; addr = addr->ai_next)
    // {
    //     if (connect_to_addr((struct sockaddr*)(addr->ai_sockaddr), addr->ai_addrlen) != 0)
    //         break;
    // }

    serv_addr.sin_family = AF_INET;
    int port = 1883;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    // int ok = inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    int ok = inet_pton(AF_INET, ipaddr, &serv_addr.sin_addr);
    if (ok <= 0)
    {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }

    client_fd = connect(sock, (struct sockaddr *)&serv_addr,
                        sizeof(serv_addr));

    if (client_fd < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    // string hello = "GET / HTTP/1.1\r\nHost: 192.168.86.32r\n\r\n";
    // char buffer[1024] = {0};

    // send it a connect
    // send it a connect
    // send it a connect
    // send it a connect
    slice s;
    mqttPacketPieces congen;
    congen.reset();

    // mqttBuffer1024 buffer;
    char assemblyArray[2048];
    sink assemblyBuffer(assemblyArray, 2048);

    mqttBuffer1024 result; // just a buffer
    sinkDrain connDestination;
    connDestination.dest = result.getSink(); // set up a sink that writes to the buffer

    slice clientID = slice("client-id-WTF");
    slice userName = slice("al4n");
    slice passWord = slice(password); // is huge

    bool connfail = congen.outputConnect(assemblyBuffer, &connDestination, clientID, userName, passWord);
    if (connfail)
    {
        cout << "generate connect failed\n";
    }
    slice theConnectBytes = connDestination.dest.getWritten();

    int slen = send(sock, theConnectBytes.charPointer(), theConnectBytes.length(), 0);
    cout << "Connect send sent " << slen << "\n";

    // // send a subscribe
    // // send a subscribe
    // // send a subscribe
    // // send a subscribe
    sendSubscribe(sock);

    // // slice s;
    // mqttPacketPieces subscribe;
    // subscribe.reset();

    // subscribe.PacketID = 2;
    // // add props?
    // // payload is the topic
    // subscribe.TopicName = slice("testtopic");
    // subscribe.UserKeyVal[0] = slice("key1");
    // subscribe.UserKeyVal[1] = slice("val1");
    // subscribe.UserKeyVal[2] = slice("key2");
    // subscribe.UserKeyVal[3] = slice("val2");
    // subscribe.QoS = 1;
    // subscribe.packetType = CtrlSubscribe;

    // // mqttBuffer1024 buffer;
    // // sink assemblyBuffer = buffer.getSink();

    // // mqttBuffer1024 result;
    // sinkDrain subDestinationBytes;
    // subDestinationBytes.src = result.getSink();

    // bool subFail = subscribe.outputPubOrSub(assemblyBuffer, &subDestinationBytes);

    // slice theSubBytes = subDestinationBytes.src.getWritten();

    // slen = send(sock, theSubBytes.charPointer(), theSubBytes.length(), 0);
    // cout << " subscribe send sent " << slen << "\n";

    // sleep(5);

    // int sval = select (int nfds, fd_set *read-fds, fd_set *write-fds, fd_set *except-fds, struct timeval *timeout);
    // printf("sval is %i\n", sval);

    // if (ioctlsocket(sock, FIONBIO, &nonblocking) != 0)
    //     DieWithError("ioctlsocket() failed");

    mqttPacketPieces parser;

    char fatbuffer[4096];
    sink collected(fatbuffer, 4096);

    unsigned int lastTime = millis();

    while (1)
    {
        char c = 0;
        int n = recv(sock, &c, 1, MSG_DONTWAIT); // async !!!
        // n == 1 means we got some
        // n == 0 mean socket was closed
        // n == -1 means no data received

        if (n == 1)
        {
            // cout << c << "\n";
            collected.writeByte(c);
            slice avail(collected);
            avail.start = 0;
            avail.end = collected.start;

            slice remains = onePacketSimpleExample(parser, avail);
            if (remains.start == avail.start)
            { // nothing happened.
            }
            else
            {
                void *dst = collected.base;
                void *src = collected.base + collected.start;
                size_t n = remains.size();
                memccpy(dst, src, n, 1);
                collected.start = 0;
            }
        }
        else if (n == 0)
        {
            break; // we disconnected
        }
        else if (n == -1)
        {
            // nothing.
        }
        else
        {
            cout << "The weird Value of 'n' is " << n << endl;
        }

        unsigned int now = millis();
        if ( (now - lastTime) > 10 * 60 * 1000 ) // 10 min
        {
            lastTime = now;
            sendSubscribe(sock);
        }
    }

    cout << "Closing now\n";

    // closing the connected socket
    close(client_fd);
    return 0;
}


bool sendSubscribe( int  sock ) {
        // send a subscribe
    // send a subscribe
    // send a subscribe
    // send a subscribe

    // slice s;
    mqttPacketPieces subscribe;
    subscribe.reset();

    subscribe.PacketID = 2;
    // add props?
    // payload is the topic
    subscribe.TopicName = slice("testtopic");
    subscribe.UserKeyVal[0] = slice("key1");
    subscribe.UserKeyVal[1] = slice("val1");
    subscribe.UserKeyVal[2] = slice("key2");
    subscribe.UserKeyVal[3] = slice("val2");
    subscribe.QoS = 1;
    subscribe.packetType = CtrlSubscribe;

    mqttBuffer1024 result ;

    char assemblyArray[2048];
    sink assemblyBuffer(assemblyArray, 2048);

    // mqttBuffer1024 result;
    sinkDrain subDestinationBytes;
    subDestinationBytes.dest = result.getSink();

    bool subFail = subscribe.outputPubOrSub(assemblyBuffer, &subDestinationBytes);

    slice theSubBytes = subDestinationBytes.dest.getWritten();

    int slen = send(sock, theSubBytes.charPointer(), theSubBytes.length(), 0);
    cout << " subscribe send sent " << slen << "\n";

    return true;
}
// onePacketSimpleExample attempts to parse a packet from availableNow
// returns a slice with what's left over after consuming packet, if any.
// that means the return is the same as the input if no packet was passed
slice onePacketSimpleExample(mqttPacketPieces &parser, const slice availableNow)
{
    using namespace knotfree;

    slice position(availableNow);

    if (position.size() < 2)
    {
        return availableNow;
    }
    int startPos = position.start;
    const unsigned char packetType = position.readByte();
    const int len = position.getLittleEndianVarLenInt();
    if (len == -1)
    { // not enough bytes
        return availableNow;
    }
    if (position.size() < len)
    { // there's not enough bytes in availableNow for this packet
        return availableNow;
    }

    parser.reset();
    bool fail = parser.parse(position, packetType, len);
    if (fail)
    { // I don't know that we can recover from this.
        cout << "FAIL in parse \n";
        // consume everything. Hope for the best.
        position.start = position.end;
        return position;
    }

    if ((packetType >> 4) == CtrlPublish)
    {

        slice tmp = availableNow;
        tmp.start = startPos;
        tmp.end = position.start + len;
        char somehex[4096];
        char *hex = tmp.gethexstr(somehex, 4096);
        // cout << hex << "\n"; // show the packet in hex
        // it was a pub
        // cout << "got pub\n";
        // expecting msg#clientId-ws131u1ewt_258 len = 27
        char namebuff[1024];
        char *nameP = parser.TopicName.getCstr(namebuff, 1024);
        cout << "on topic " << nameP;
        char msgbuff[1024];
        char *msgP = parser.Payload.getCstr(msgbuff, 1024);
        cout << " got message " << msgP;
        char *resP = parser.RespTopic.getCstr(msgbuff, 1024);
        cout << " respond to " << resP;
        char *corrP = parser.CorrelationData.gethexstr(msgbuff, 1024);
        cout << " correlation " << corrP;
        cout << "\n";
    }
    else
    {
        cout << "got a packet " << (packetType >> 4) << "\n";
    }

    position.start += len;

    return position;
}

#include <chrono>

unsigned int millis() {
	using namespace std::chrono;
	return static_cast<uint32_t>(duration_cast<milliseconds>(
		system_clock::now().time_since_epoch()).count());
}
