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
#include <vector>

#include <arpa/inet.h>
#include <iostream>

using namespace std;

#define MAXDATASIZE 10000 // max number of bytes we can get at once
#define BACKLOG 10   // how many pending connections queue will hold
#define LOCALHOST "localhost"
#define APORT "30086"
#define BPORT "31086"
#define UDPPORT "32086"
#define TCPPORT "33086"  // the port users will be connecting to
#define NOTFOUND "NF"

void *get_in_addr(struct sockaddr *sa);
in_port_t get_in_port(struct sockaddr *sa);
vector<string> split(string s, string delimiter);


/**
 * Split the input string into an array of strings seperated by delimiter.
 * Source @ https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
 * @param s input string to be split.
 * @param delimiter delimiter to split strings.
 * @return a vector of strings split by the delimiter.
 */
vector<string> split(string s, string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}


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


/**
 * Get socket port number, IPv4 or IPv6:
 * @param sa socket address
 * @return socket port number
 */
in_port_t get_in_port(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return (((struct sockaddr_in *) sa)->sin_port);
    }

    return (((struct sockaddr_in6 *) sa)->sin6_port);
}


int main(int argc, char *argv[]) {
    // code for a client connecting to a server.
    // namely a stream socket to localhost on TCPPORT.
    // source: @ https://beej.us/guide/bgnet/html/split/man-pages.html#acceptman
    int sockfd, numbytes;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET_ADDRSTRLEN];
    char buf[MAXDATASIZE];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = PF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(LOCALHOST, TCPPORT, &hints, &servinfo)) != 0) {
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

    cout << "Client is up and running" << endl;

    // Get my port
    struct sockaddr_in my_addr;
    bzero(&my_addr, sizeof(my_addr));
    socklen_t len = sizeof(my_addr);
    getsockname(sockfd, (struct sockaddr *) &my_addr, &len);
    unsigned int myPort = ntohs(my_addr.sin_port);

    string input_dname;
    string input_id;
    string input;
    const char *inputCstr;

    while (1) {
        memset(buf, '\0', sizeof(buf));

        cout << "Enter department Name: ";
        cin >> input_dname;  // getline(cin, input);
        cout << "Enter student ID: ";
        cin >> input_id;  // getline(cin, input);
        input = input_dname + "," + input_id;
        inputCstr = input.c_str();

        // send()
        if (send(sockfd, inputCstr, MAXDATASIZE - 1, 0) == -1) {
            perror("send");
            exit(EXIT_SUCCESS);
        }

        printf("Client has sent %s and Student %s to Main Server using TCP over port %u\n", input_dname.c_str(), input_id.c_str(), myPort);


        // receive()
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
            perror("recv");
            exit(1);
        }

        buf[numbytes] = '\0';

//        cout << "buf is: " << buf << endl;

        if (buf[0] == '-') { // received message starting with "-" means department not found in main server
            printf("%s: Not found\n", input_dname.c_str());
        } else if (buf[0] == '/') { // received message starting with "/" means student id not found in backend server
            printf("Student %s: Not found\n", input_id.c_str());
        } else {
            vector<string> results = split(buf, ",");
            printf("The performance statistics for Student %s in Department %s is:\n"
                   "Student GPA: %s\n"
                   "Percentage Rank: %s\n"
                   "Department GPA Mean: %s\n"
                   "Department GPA Variance: %s\n"
                   "Department Max GPA: %s\n"
                   "Department Min GPA: %s\n"
                   "Friend Recommendation: %s\n",
                   input_id.c_str(), input_dname.c_str(),
                   results[0].c_str(),
                   results[1].c_str(),
                   results[2].c_str(),
                   results[3].c_str(),
                   results[4].c_str(),
                   results[5].c_str(),
                   results[6].c_str());
        }

        cout << "\n-----Start a new request-----" << endl;
    }

    close(sockfd);

    return 0;
}