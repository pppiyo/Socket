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
#include <algorithm>
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
string getIdncnt(string depart, unordered_map <string, vector<string> > mp);
vector <string> getDistinct(vector <string> v);
vector <string> split(string s, string delimiter);
unordered_map <string, vector<string> > storeInfo(string filename); // {ECE: [113,1236,..], CS: [8,21,2220,..]}
vector<string> makedlist(unordered_map <string, vector<string> > mp);
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
 * Get a string of student ids corresponding to the given department name, ending with distinct id count.
 * @param depart department name.
 * @param mp unordered map of department name - student ids pairs.
 * @return a string of student ids corresponding to the given department name, ending with distinct id count. Seperated by commas.
 */
string getIdncnt(string depart, unordered_map <string, vector<string> > mp) {
	unordered_map <string, vector<string> >::iterator it;
	for (it = mp.begin(); it != mp.end(); ++it) {
		if (depart == it->first.substr(0, it->first.length() - 1)) {
			return concatVect(it->second);
		}
	}

	return NOTFOUND;
}


/**
 * Get a string vector of distinct student ids.
 * @param v given a vector of read id strings, find distinct ones and use them to form a new vector.
 * @return a vector of distinct elements of the given vector.
 */
vector <string> getDistinct(vector <string> v) {
  set<int> ids;
  for (int i = 0; i < v.size(); i++) {
    ids.insert(stoi(v[i]));
  }

  vector <string> result;

  for (auto it = ids.begin(); it != ids.end(); it++) {
      stringstream ss;
      ss << *it;
      string str = ss.str(); 
      result.push_back(str);
  }

  stringstream ss;
  ss << ids.size();
  string str = ss.str();
  result.push_back(str);

  return result;
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
 * Server A read the file and store the information.
 * @param filename file to read from.
 * @return a map of department names with their associated student ids plus counted distinct number to the end.
 */
unordered_map <string, vector<string> > storeInfo(string filename) {
    unordered_map<string, vector < string> > result;
    fstream datafile;
    datafile.open(filename, ios::in);

    if (datafile.is_open()) {
        string input;
        string name;
        vector <string> idsncnt; // ids and count

        while (getline(datafile, input)) {
            name = input;
            getline(datafile, input);
            idsncnt = split(input, ",");

            vector<string> distIds = getDistinct(idsncnt);
            result.insert(std::make_pair(name, distIds));
        }
    }

    return result;
}


/**
 * Make a list of department names from the department name - student ids pairs. Store in a string vector.
 * @param mp unordered map of department name - student ids pairs.
 * @return make a list of department names, store in a string vector.
 */
vector<string> makedlist(unordered_map <string, vector<string> > mp) {
    vector<string> keys;
    for (unordered_map<string, vector<string> >::iterator it = mp.begin(); it != mp.end(); it++) {
        keys.push_back(it->first.substr(0, it->first.length()-1));
    }

    return keys;
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
    // receive params
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET_ADDRSTRLEN];

    // send params
    int sock_snd;
    struct addrinfo hints_snd, *servinfo_snd, *p_snd;
    int rv_snd;
    int numbytes_snd;


    // read the file dataA.txt and store the information in map structure "dict".
    unordered_map<string, vector < string> > dict = storeInfo("dataA.txt");
    vector<string> dlist = makedlist(dict);


    // establish receive socket;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(LOCALHOST, APORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("serverA: socket");
            continue;
        }

        if (::bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("serverA: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "serverA: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);


    // establish send socket
    memset(&hints_snd, 0, sizeof hints_snd);
    hints_snd.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints_snd.ai_socktype = SOCK_DGRAM;

    if ((rv_snd = getaddrinfo(LOCALHOST, MAINPORT, &hints_snd, &servinfo_snd)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_snd));
        return 1;
    }

    for (p_snd = servinfo_snd; p_snd != NULL; p_snd = p_snd->ai_next) {
        if ((sock_snd = socket(p_snd->ai_family, p_snd->ai_socktype,
                               p_snd->ai_protocol)) == -1) {
            perror("serverA: socket");
            continue;
        }

        break;
    }

    if (p_snd == NULL) {
        fprintf(stderr, "serverA: failed to create socket\n");
        return 2;
    }


    // print boot up messages.
    printf("Server A is up and running using UDP on port %s\n", APORT);


    // receive main server query
    memset(&buf, 0, sizeof buf);
    addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *) &their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }

    if (buf[0] == '!') {
        string tmp = "A";
		// send department list to main server.
        for (size_t i = 0; i < dlist.size(); i++) {
        	tmp.append(",");
            tmp.append(dlist[i]);
        }

        if ((numbytes_snd = sendto(sock_snd, tmp.c_str(), strlen(tmp.c_str()), 0, p_snd->ai_addr, p_snd->ai_addrlen)) == -1) {
            perror("serverA: sendto");
            exit(1);
        }

        // printf("This is what it has sent.%s", tmp.c_str());
        printf("Server A has sent a department list to Main Server\n");
    }


    // receive actions
    while (1) {
	    // receive main server query
	    memset(&buf, 0, sizeof buf);
	    addr_len = sizeof their_addr;

	    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *) &their_addr, &addr_len)) == -1) {
	        perror("recvfrom");
	        exit(1);
	    }

        if (buf[0] != '!') {
            printf("Server A has received a request for %s\n", buf);
            // find the department and student IDs
            string depart(buf);
            string idncnt = getIdncnt(depart, dict);
            vector<string> vect = split(idncnt, ",");
            string cntstr = vect[vect.size() - 1];
            vect.pop_back();
            string idstr = concatVect(vect);
            printf("Server A found %s distinct students for %s: %s\n", cntstr.c_str(), depart.c_str(), idstr.c_str());
            string result = "A" + idncnt;

            // send results back to main server
            if ((numbytes_snd = sendto(sock_snd, result.c_str(), strlen(result.c_str()), 0, p_snd->ai_addr, p_snd->ai_addrlen)) == -1) {
              perror("serverA: sendto");
              exit(1);
            }

            printf("Server A has sent the results to Main Server\n\n");
        }
    }

    close(sockfd);
    close(sock_snd);

    return 0;
}