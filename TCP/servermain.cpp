//
// Created by Amy Lee on 2/17/23.
//

/**
 * Adapted from server.c -- a stream socket server demo
 * Reference: https://beej.us/guide/bgnet/html
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <set>

using namespace std;

#define PORT "33086"  // the port users will be connecting to
#define BACKLOG 10   // how many pending connections queue will hold
#define MAXDATASIZE 100 // max number of bytes we can get at once
#define NOTFOUND (-1)

vector<string> split(string s, string delimiter);

map<int, vector<string> > storeInfo(string filename);

int countDistinct(vector<string> v);

void printCountInfo(map<int, vector<string> > mp);

int lookupServerID(map<int, vector<string> > mp, string name);

void sigchld_handler(int s);

void *get_in_addr(struct sockaddr *sa);

in_port_t get_in_port(struct sockaddr *sa);

void printNotFoundMsg(map<int, vector<string> > mp, string name);


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
 * Main server read the file and store the information.
 * @param filename file to read from.
 * @return a map of backend server id with its asscociated department names.
 */
map<int, vector<string> > storeInfo(string filename) {
    map<int, vector<string> > result;
    fstream datafile;
    datafile.open(filename, ios::in);

    if (datafile.is_open()) {
        string input;

        int id = -1;
        vector<string> names;

        while (getline(datafile, input)) {
            id = stoi(input);
            getline(datafile, input);
            names = split(input, ",");
            result.insert(std::make_pair(id, names));
        }
    }

    return result;
}


/**
 * Count number of distinct elements in a vector.
 * @param v input vector.
 * @return number of distinct elements.
 */
int countDistinct(vector<string> v) {
    set<string> s(v.begin(), v.end());
    return s.size();
}


/**
 * Print the counting results of which department a backend server is responsible for. Repeated departments are counted only once.
 * @param mp input map structure created from list.txt which contains all server ID and the corresponding department information.
 */
void printCountInfo(map<int, vector<string> > mp) {
    cout << "Total number of Backend Servers: " << mp.size() << endl;
    map<int, vector<string> >::iterator it;
    for (it = mp.begin(); it != mp.end(); ++it) {
        cout << "Backend Servers " << it->first << " contains " << countDistinct(it->second) << " distinct departments"
             << endl;
    }
}


/**
 * Lookup and return corresponding backend server id for the given name. Return NOTFOUND if id is not found.
 * @param mp input map structure created from list.txt which contains all server ID and the corresponding department information.
 * @param name department name to look for in the map.
 * @return Server id corresponding to the given name.
 */
int lookupServerID(map<int, vector<string> > mp, string name) {
    map<int, vector<string> >::iterator it;

    for (it = mp.begin(); it != mp.end(); ++it) {
        int len = it->second.size();
        for (int i = 0; i < len; i++) {
            if (it->second[i].compare(name) == 0) {
                return it->first;
            }
        }
    }

    return NOTFOUND;
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


/**
 * Print "not found" message if name was not found in map.
 * @param mp input server information list.
 * @param name name to look up in server information list.
 */
void printNotFoundMsg(map<int, vector<string> > mp, string name) {
    cout << name << " does not show up in backend server ";
    map<int, vector<string> >::iterator it;

    for (it = mp.begin(); it != mp.end(); ++it) {
        if (it == mp.begin()) {
            cout << it->first;
        } else {
            cout << ", " << it->first;
        }
    }

    cout << "." << endl;
}


int main(void) {
    // Print boot messages.
    cout << "Main server is up and running." << endl;

    // Main server read the file list.txt and store the information in map structure "dict".
    map<int, vector<string> > dict = storeInfo("list.txt");
    cout << "Main server has read the department list from list.txt." << endl;

    // Print the counting results of which department a backend server is responsible for.
    printCountInfo(dict);

    // Main server process wait for client processes to connect.
    // namely a stream socket on port 33086, on this host's IP.
    // source: @ https://beej.us/guide/bgnet/html/split/man-pages.html#acceptman
    int sockfd, new_fd, numbytes;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    char buf[MAXDATASIZE];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // socket(), bind()
    for (p = servinfo; p != NULL; p = p->ai_next) { // loop through all the results and bind to the first we can
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break; // if we get here, we must have connected successfully.
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    // listen()
    if (listen(sockfd, BACKLOG) == -1) {
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
    int cnt = 0;

    while (1) {  // main accept() loop
        cnt += 1;
        sin_size = sizeof their_addr;

        // accept()
        new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener

            while (1) {
                memset(buf, '\0', sizeof(buf));

                // receive()
                if ((numbytes = recv(new_fd, buf, MAXDATASIZE - 1, 0)) == -1) {
                    perror("recv");
                    exit(1);
                }

                buf[numbytes] = '\0';

                printf("Main server has received the request on Department %s from client %d using TCP over port %s\n",
                       buf, cnt, PORT);

                // lookup
                string name = buf;
                int id = lookupServerID(dict, name);
                string tmp = to_string(id);
                char const *serverID = tmp.c_str();

                if (id == NOTFOUND) {
                    printNotFoundMsg(dict, name);
                } else {
                    printf("%s shows up in backend server %d\n", name.c_str(), id);
                }

                // send()
                if (send(new_fd, serverID, MAXDATASIZE - 1, 0) == -1) {
                    perror("send");
                }

                if (id == NOTFOUND) {
                    printf("The Main Server has sent \"Department Name: Not found\" to client %d using TCP over port %s\n\n",
                           cnt, PORT);
                } else {
                    printf("Main Server has sent searching result to client %d using TCP over port %s\n\n", cnt, PORT);
                }
            }

            close(new_fd);  // parent doesn't need this
            exit(0);
        }
    }

    return 0;
}