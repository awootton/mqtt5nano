

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
#include <unistd.h>

using namespace std;

int main()
{
    // this is an example but not a good test because it relies
    // on a tiny server running on 192.168.86.32

    cout << "Hello World! test tcp async \n";

    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    int port = 80;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    int ok = inet_pton(AF_INET, "192.168.86.32", &serv_addr.sin_addr);
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

    string hello = "GET / HTTP/1.1\r\nHost: 192.168.86.32r\n\r\n";
    char buffer[1024] = {0};

    int slen = send(sock, hello.c_str(), hello.length(), 0);
    printf("Hello message sent\n");

    // sleep(5);

    // int sval = select (int nfds, fd_set *read-fds, fd_set *write-fds, fd_set *except-fds, struct timeval *timeout);
    // printf("sval is %i\n", sval);

    // if (ioctlsocket(sock, FIONBIO, &nonblocking) != 0)
    //     DieWithError("ioctlsocket() failed");

    int buffpos = 0;
    while (1)
    {
        char c = 0;
        int n = recv(sock, &c, 1, MSG_DONTWAIT); // async !!!
        // n == 1 means we got some
        // n == 0 mean socket was closed
        // n == -1 means no data received

        if (n == 1)
        {
            cout << c;
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
            cout<<"The weird Value of 'n' is "<< n << endl;
        }

        // sleep(1);
        // continue;

        // if (n == 1)
        // {
        //     if ( buffpos >= sizeof(buffer)){
        //         printf("ERROR overflow buffer");
        //         break;
        //     }
        //     buffer[buffpos++] = c;
        //     if (c == '\n' || c == 'r')
        //     {
        //         //break;
        //     }
        //     cout << c;
        // }
        // else if (n < 0)
        // {
        //     printf("error out of loop\n");
        //     sleep(1);
        //     break;
        // }
    }

    //  int n = recv(sock, buffer, 1024, MSG_DONTWAIT);
    //  printf("recv got %s ", buffer);
    //  cout<<"The Value of 'num' is "<< n << endl;

    //  valread = read(sock, buffer, 1024);
    //printf("%s\n", buffer);

    cout << "Closing now\n";

    // closing the connected socket
    close(client_fd);
    return 0;
}
