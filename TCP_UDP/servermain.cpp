#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <unordered_map>
#include <set>
#include <numeric>
#include <string.h>
#include <signal.h>

#include "Department.h"
#include "Student.cpp"


using namespace std;

#define MAXDATASIZE 10000 // max number of bytes we can get at once
#define BACKLOG 10   // how many pending connections queue will hold
#define LOCALHOST "localhost"
#define APORT "30086"
#define BPORT "31086"
#define UDPPORT "32086"
#define TCPPORT "33086"  // the port users will be connecting to
#define NOTFOUND "NF"

vector <string> split(string s, string delimiter);

void *get_in_addr(struct sockaddr *sa);

in_port_t get_in_port(struct sockaddr *sa);

void sigchld_handler(int s);

char lookUpServerID(vector <string> Alist, vector <string> Blist, string rdname);

vector <string> split(string s, string delimiter);


// for debugging
void printVector(vector <string> v) {
    for (int i = 0; i < v.size(); i++) {
        cout << v[i] << endl;
    }
}


void printVector(vector<int> v) {
    for (int i = 0; i < v.size(); i++) {
        cout << v[i] << " ";
    }
    cout << endl;
}


char lookUpServerID(vector <string> Alist, vector <string> Blist, string rdname) {
    char res = '-'; // result to return

    if (std::find(Alist.begin(), Alist.end(), rdname) != Alist.end()) {
        res = 'A';
    } else if (std::find(Blist.begin(), Blist.end(), rdname) != Blist.end()) {
        res = 'B';
    }

    return res;
}


/**
 * Split the input string into an array of strings seperated by delimiter.
 * Source @ https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
 * @param s input string to be split.
 * @param delimiter delimiter to split strings.
 * @return a vector of strings split by the delimiter.
 */
vector <string> split(string s, string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector <string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}


/**
 * Kill all zombie processes.
 */
void sigchld_handler(int s) {
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
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


int main(void) {
    // Print boot up messages.
    cout << "Main server is up and running." << endl;

    /* UDP establishment. */
    // send params for socket A
    int sock_snd_A;
    struct addrinfo hints_snd_A, *servinfo_snd_A, *p_snd_A;
    int rv_snd_A;
    int numbytes_snd_A;

    // send params for socket B
    int sock_snd_B;
    struct addrinfo hints_snd_B, *servinfo_snd_B, *p_snd_B;
    int rv_snd_B;
    int numbytes_snd_B;

    // receive params
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXDATASIZE];
    socklen_t addr_len;
    char s[INET_ADDRSTRLEN];
    vector <string> Alist;
    vector <string> Blist;


    // establish send socket A
    memset(&hints_snd_A, 0, sizeof hints_snd_A);
    hints_snd_A.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints_snd_A.ai_socktype = SOCK_DGRAM;

    if ((rv_snd_A = getaddrinfo(LOCALHOST, APORT, &hints_snd_A, &servinfo_snd_A)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_snd_A));
        return 1;
    }

    for (p_snd_A = servinfo_snd_A; p_snd_A != NULL; p_snd_A = p_snd_A->ai_next) {
        if ((sock_snd_A = socket(p_snd_A->ai_family, p_snd_A->ai_socktype,
                                 p_snd_A->ai_protocol)) == -1) {
            perror("servermain: socket");
            continue;
        }

        break;
    }

    if (p_snd_A == NULL) {
        fprintf(stderr, "servermain: failed to create socket A\n");
        return 2;
    }


    // establish send socket B
    memset(&hints_snd_B, 0, sizeof hints_snd_B);
    hints_snd_B.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints_snd_B.ai_socktype = SOCK_DGRAM;

    if ((rv_snd_B = getaddrinfo(LOCALHOST, BPORT, &hints_snd_B, &servinfo_snd_B)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_snd_B));
        return 1;
    }

    for (p_snd_B = servinfo_snd_B; p_snd_B != NULL; p_snd_B = p_snd_B->ai_next) {
        if ((sock_snd_B = socket(p_snd_B->ai_family, p_snd_B->ai_socktype,
                                 p_snd_B->ai_protocol)) == -1) {
            perror("servermain: socket");
            continue;
        }

        break;
    }

    if (p_snd_B == NULL) {
        fprintf(stderr, "servermain: failed to create socket B\n");
        return 2;
    }

    // establish receive socket;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(LOCALHOST, UDPPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("servermain: socket");
            continue;
        }

        if (::bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("servermain: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "servermain: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);


    // send query to both A & B
    char query[] = "!";
    if ((numbytes_snd_A = sendto(sock_snd_A, query, strlen(query), 0,
                                 p_snd_A->ai_addr, p_snd_A->ai_addrlen)) == -1) {
        perror("servermain: sendto");
        exit(1);
    }

    if ((numbytes_snd_B = sendto(sock_snd_B, query, strlen(query), 0,
                                 p_snd_B->ai_addr, p_snd_B->ai_addrlen)) == -1) {
        perror("servermain: sendto");
        exit(1);
    }

    while (1) {
        // receive department list
        memset(&buf, 0, sizeof buf);
        addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE - 1, 0, (struct sockaddr *) &their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        buf[numbytes] = '\0';
        string bufstr(buf);

        if (bufstr[0] == 'A') {
            Alist = split(bufstr.substr(2, bufstr.length()), ",");
            printf("Main server has received the department list from server A using UDP over port %s\n", UDPPORT);

            if (Alist.size() > 0) {
                printf("ServerA\n");
                printVector(Alist);
            }
        }

        if (bufstr[0] == 'B') {
            Blist = split(bufstr.substr(2, bufstr.length()), ",");
            printf("Main server has received the department list from server B using UDP over port %s\n", UDPPORT);

            if (Blist.size() > 0) {
                printf("ServerB\n");
                printVector(Blist);
            }
        }

        cout << endl;

       if (Alist.size() > 0 && Blist.size() > 0) { // todo: after serverB is done, correct to this
        // if (Alist.size() > 0 || Blist.size() > 0) {
            break;
        }
    }

    /* Establish TCP socket. */
    // Main server process wait for client processes to connect.
    // namely a stream socket on port 33086, on this host's IP.
    // source: @ https://beej.us/guide/bgnet/html/split/man-pages.html#acceptman
    int sockfd_tcp, new_fd, numbytes_tcp;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints_tcp, *servinfo_tcp, *p_tcp;
    struct sockaddr_storage their_addr_tcp; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s_tcp[INET_ADDRSTRLEN];
    int rv_tcp;
    char buf_tcp[MAXDATASIZE];

    memset(&hints_tcp, 0, sizeof hints_tcp);
    hints_tcp.ai_family = PF_INET;
    hints_tcp.ai_socktype = SOCK_STREAM;
    hints_tcp.ai_flags = AI_PASSIVE; // use my IP

    if ((rv_tcp = getaddrinfo(NULL, TCPPORT, &hints_tcp, &servinfo_tcp)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_tcp));
        return 1;
    }

    // socket(), bind()
    for (p_tcp = servinfo_tcp;
         p_tcp != NULL; p_tcp = p_tcp->ai_next) { // loop through all the results and bind to the first we can
        if ((sockfd_tcp = socket(p_tcp->ai_family, p_tcp->ai_socktype,
                                 p_tcp->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd_tcp, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (::bind(sockfd_tcp, p_tcp->ai_addr, p_tcp->ai_addrlen) == -1) {
            close(sockfd_tcp);
            perror("server: bind");
            continue;
        }

        break; // if we get here, we must have connected successfully.
    }

    freeaddrinfo(servinfo_tcp); // all done with this structure

    if (p_tcp == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    // listen()
    if (listen(sockfd_tcp, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }


    // accept(), send(), receive()
    int clientId = 0;

    while (1) {  // main accept() loop
        clientId += 1;
        sin_size = sizeof their_addr_tcp;

        // accept()
        new_fd = accept(sockfd_tcp, (struct sockaddr *) &their_addr_tcp, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        if (!fork()) { // this is the child process
            close(sockfd_tcp); // child doesn't need the listener

            while (1) {
                memset(buf_tcp, '\0', sizeof(buf_tcp));

                // receive()
                if ((numbytes_tcp = recv(new_fd, buf_tcp, MAXDATASIZE - 1, 0)) == -1) {
                    perror("recv");
                    exit(1);
                }

                buf_tcp[numbytes_tcp] = '\0';
                vector <string> request = split(buf_tcp, ",");

                printf("Main server has received the request on Student %s in %s from client %d using TCP over port %s\n",
                       request[1].c_str(), request[0].c_str(), clientId, TCPPORT);

                // lookup & send (thru UDP to A/B if found match or thru TCP to client if otherwise)
                string rdname = request[0]; // rdname short for requested department name
                string rid = request[1]; // rid short for requested (student) id
                string toSend = rdname + "," + rid;

                char serverID = lookUpServerID(Alist, Blist, rdname);

                if (serverID == '-') {
                    printf("%s does not show up in server A&B\n", rdname.c_str());

                    // TCP send() not found
                    if (send(new_fd, "-", MAXDATASIZE - 1, 0) == -1) {
                        perror("send");
                    }

                    printf("Main Server has sent \"%s: Not found\" to client %d using TCP over port %s\n\n",
                           rdname.c_str(), clientId, TCPPORT);

                } else { // if found in A/B
                    printf("%s shows up in backend server %c\n", rdname.c_str(), serverID);

                    if (serverID == 'A') { // if found in A
                        // UDP send() to A
                        if ((numbytes_snd_A = sendto(sock_snd_A, toSend.c_str(), strlen(toSend.c_str()), 0,
                                                     p_snd_A->ai_addr,
                                                     p_snd_A->ai_addrlen)) == -1) {
                            perror("servermain: sendto");
                            exit(1);
                        }

                    } else { // if found in B
                        // UDP send() to B
                        if ((numbytes_snd_B = sendto(sock_snd_B, toSend.c_str(), strlen(toSend.c_str()), 0,
                                                     p_snd_B->ai_addr,
                                                     p_snd_B->ai_addrlen)) == -1) {
                            perror("servermain: sendto");
                            exit(1);
                        }
                    }

                    printf("Main Server has sent request of Student %s to server A/B using UDP over port %s\n\n",
                           rid.c_str(), UDPPORT);

                    // receive results thru UDP from server A or B
                    memset(&buf, 0, sizeof buf);

                    // addr_len = sizeof their_addr;
                    if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE - 1, 0, (struct sockaddr *) &their_addr,
                                             &addr_len)) == -1) {
                        perror("recvfrom");
                        exit(1);
                    }

                    buf[numbytes] = '\0';
                    string results(buf);
                    string validInfo;

                    if (buf[2] == '/') {
                        printf("Main server has received \"Student %s: Not found\" from server %c\n",
                               rid.c_str(), serverID);

                        validInfo = '/';

                        // TCP send() student id not found
                        if (send(new_fd, validInfo.c_str(), MAXDATASIZE - 1, 0) == -1) {
                            perror("send");
                        }

                        printf("Main Server has sent message to client %d using TCP over %s\n\n",
                               clientId, TCPPORT);

                    } else {
                        printf("Main server has received searching result of Student %s from server %c\n",
                               rid.c_str(), buf[0]);

                        validInfo = results.substr(2, results.length() - 1);

                        // TCP send() student info to client // UDP message with "A/B," removed
                        if (send(new_fd, validInfo.c_str(), MAXDATASIZE - 1, 0) == -1) {
                            perror("send");
                        }

                        printf("Main Server has sent searching result(s) to client %d using TCP over port %s\n\n",
                               clientId, TCPPORT);
                    }
                }
            }

            close(new_fd);  // parent doesn't need this
            exit(0);
        }
    }

    return 0;
}