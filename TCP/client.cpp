//
// Created by Amy Lee on 2/17/23.
//

/**
 * Adapted from client.c -- a stream socket client demo
 * Reference: https://beej.us/guide/bgnet/html
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <arpa/inet.h>
#include <iostream>

using namespace std;

#define PORT "33086" // the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once

void *get_in_addr(struct sockaddr *sa);

/**
 * Get socket address, IPv4 or IPv6.
 * @param sa socket address
 * @return socket address
 */
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}


int main(int argc, char *argv[]) {
    // code for a client connecting to a server.
    // namely a stream socket to localhost on port 33086.
    // source: @ https://beej.us/guide/bgnet/html/split/man-pages.html#acceptman
    int sockfd, numbytes;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    char buf[MAXDATASIZE];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = PF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // socket(), connect()
    for (p = servinfo; p != NULL; p = p->ai_next) { // loop through all the results and connect to the first we can
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    freeaddrinfo(servinfo); // all done with this structure

    cout << "Client is up and running." << endl;

    string input;
    const char *inputCstr;

    while (1) {
        memset(buf, '\0', sizeof(buf));

        cout << "Enter Department Name: ";
        cin >> input;  // getline(cin, input);
        inputCstr = input.c_str();

        // send()
        if (send(sockfd, inputCstr, MAXDATASIZE - 1, 0) == -1) {
            perror("send");
            exit(EXIT_SUCCESS);
        }

        printf("Client has sent Department %s to Main Server using TCP.\n", inputCstr);

        // receive()
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
            perror("recv");
            exit(1);
        }

        buf[numbytes] = '\0';

        if (buf[0] == '-') {
            printf("%s not found.\n", inputCstr);
        } else {
            printf("Client has received results from Main Server:\n%s is associated with backend server %s.\n",
                   inputCstr, buf);
        }

        cout << "\n-----Start a new query-----" << endl;
    }

    close(sockfd);

    return 0;
}