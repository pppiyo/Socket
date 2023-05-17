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
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <set>

#define LOCALHOST "localhost"
#define APORT "30086"
#define BPORT "31086"
#define MAINPORT "32086"
#define MAXBUFLEN 10000
#define NOTFOUND "NF"

using namespace std;

string concatVect(vector<string> v);
string lookupServer(string input, vector<string> Alist, vector<string> Blist);
bool lookupList(string input, vector<string> list);
void printVector(vector <string> vect);
vector <string> split(string s, string delimiter);
string convertToString(char *a);
void *get_in_addr(struct sockaddr *sa);
in_port_t get_in_port(struct sockaddr *sa);


/**
 * Concatenate vectors strings with comma.
 * @param v input string vector.
 * @return concatenated string.
 */
string concatVect(vector<string> v) {
    string result = "";
    for (int i = 0; i < v.size(); i++) {
        result.append(v[i]);
        if (i != v.size() - 1) {
            result.append(",");
        }
    }

    return result;
}


/**
 * Given a department name and two server lists, find which server the department belongs to.
 * @param input user input department name.
 * @param Alist serverA's department list.
 * @param Blist serverA's department list.
 * @return
 */
string lookupServer(string input, vector<string> Alist, vector<string> Blist) {
    if (lookupList(input, Alist)) {
        return "A";
    }    

    if (lookupList(input, Blist)) {
        return "B";
    }

    return NOTFOUND;
}


/**
 * Find a string in a list, if it exists in the list, return true, otherwise return false.
 * @param input string to look up in the list.
 * @param list list to find the string.
 * @return true if string exists in the list, false otherwise.
 */
bool lookupList(string input, vector<string> list) {
    for (int i = 0; i < list.size(); i++) {
        if (list[i] == input) {
            return true;
        }
    }

    return false;
}


/**
 * Print elements in the given vector one by one.
 * @param vect vector whose elements to be print.
 */
void printVector(vector <string> vect) {
    for (int i = 0; i < vect.size(); i++) {
        cout << vect[i] << endl;
    }
    cout << endl;
}


/**
 * Split the input string into an array of integers seperated by delimiter.
 * Adapted from Source @ https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
 * @param s input string to be split.
 * @param delimiter delimiter to split strings.
 * @return a vector of integers split by the delimiter.
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
 * Convert a char to string.
 * @param a char to be converted to a string.
 * @return a string converted from a char.
 */
string convertToString(char *a) {
    string s(a);
    return s;
}


/**
 * Get socket address, IPv4 or IPv6:
 * @param sa socke address in sockaddr struct.
 * @return sockaddr, IPv4 or IPv6.
 */
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}


/**
 * Get socket number, IPv4 or IPv6:
 * @param sa socke address in sockaddr struct.
 * @return sockaddr, IPv4 or IPv6.
 */
in_port_t get_in_port(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return (((struct sockaddr_in *) sa)->sin_port);
    }

    return (((struct sockaddr_in6 *) sa)->sin6_port);
}


int main(void) {
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
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET_ADDRSTRLEN];

    vector<string> Alist;
    vector<string> Blist;

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

    if ((rv = getaddrinfo(LOCALHOST, MAINPORT, &hints, &servinfo)) != 0) {
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

    // main server booting up.
    printf("Main server is up and running\n");


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

    while(1) {
        // receive department list
        memset(&buf, 0, sizeof buf);
        addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *) &their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        buf[numbytes] = '\0';

        string bufstr(buf);

        if (bufstr[0] == 'A') {
            Alist = split(bufstr.substr(2, bufstr.length()), ",");
            printf("Main server has received the department list from server A using UDP over port %s\n", MAINPORT);

            if (Alist.size() > 0) {
                printf("ServerA\n");
                printVector(Alist);
            }
        }

        if (bufstr[0] == 'B') {
            Blist = split(bufstr.substr(2, bufstr.length()), ",");
            printf("Main server has received the department list from server B using UDP over port %s\n", MAINPORT);

            if (Blist.size() > 0) {
                printf("ServerB\n");
                printVector(Blist);
            }
        }

        if (Alist.size() > 0 && Blist.size() > 0) {
            break;
        }
    }

    while (1) {
        // get department name and send query (user input)
        cout << "Enter Department Name: ";
        string input;
        cin >> input;  // getline(cin, input);
        string sid;

        sid = lookupServer(input, Alist, Blist);

        if (sid == "A") {
            printf("%s shows up in server A\n", input.c_str());

            if ((numbytes_snd_A = sendto(sock_snd_A, input.c_str(), strlen(input.c_str()), 0, p_snd_A->ai_addr, p_snd_A->ai_addrlen)) == -1) {
                perror("servermain: sendto");
                exit(1);
            }

            printf("The Main Server has sent request for %s to server A using UDP over port %s\n", input.c_str(), MAINPORT);      
    
        } else if (sid == "B") {

            printf("%s shows up in server B\n", input.c_str());

            if ((numbytes_snd_B = sendto(sock_snd_B, input.c_str(), strlen(input.c_str()), 0, p_snd_B->ai_addr, p_snd_B->ai_addrlen)) == -1) {
                perror("servermain: sendto");
                exit(1);
            }  

            printf("The Main Server has sent request for %s to server B using UDP over port %s\n", input.c_str(), MAINPORT);

        } else {
            printf("%s does not show up in server A&B\n", input.c_str());
            continue; // debug
        }


        // receive searching results from server A or B
        memset(&buf, 0, sizeof buf);
        // addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *) &their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        buf[numbytes] = '\0';

        string results(buf); // A,123,432,5654

        if (buf[0] == 'A') {
            printf("The Main server has received searching result(s) of %s from server A\n", input.c_str());
        }

        if (buf[0] == 'B') {
            printf("The Main server has received searching result(s) of %s from server B\n", input.c_str());
        }

        string validInfo = results.substr(1, results.length()-1);

        vector<string> vect = split(validInfo, ",");

        string cntstr = vect[vect.size() - 1];

        vect.pop_back();

        string idstr = concatVect(vect);

        printf("There are %s distinct students in %s department. Their IDs are %s\n", cntstr.c_str(), input.c_str(), idstr.c_str());

        cout << endl;
        cout << "-----Start a new query-----" << endl;
    }

    close(sockfd);
    close(sock_snd_A);
    close(sock_snd_B);

    return 0;
}