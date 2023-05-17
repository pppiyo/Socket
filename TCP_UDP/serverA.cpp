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
#include <numeric>

#include "Department.cpp"
#include "Student.cpp"

#define LOCALHOST "localhost"
#define APORT "30086"
#define BPORT "31086"
#define MAINPORT "32086" // SET
#define MAXBUFLEN 10000

using namespace std;

string concatVect(vector <string> v);

vector <string> getKeys(unordered_multimap <string, Student> mp);

vector <string> split(string s, string delimiter);

vector <string> splitInputLine(string s, string delimiter);

unordered_multimap <string, Student>
storeInfo(string filename); // insert dname - student pairs, where dname can be the same for multiple entries.
void *get_in_addr(struct sockaddr *sa);

in_port_t get_in_port(struct sockaddr *sa);

vector <Department> makeDprtList(unordered_multimap <string, Student> dict, string dname);

string getDprtStuInfoString(Department dprt, Student stu, int friendId);

Student findStudent(vector <Department> dlist, string dname, int id);

int getDprtIndex(vector <Department> dlist, string dname);

float getCosSimilarity(vector<int> scores1, vector<int> scores2);

int findFriend(vector <Student> peers, Student stu);

vector <Student> getAllStusInOrder(unordered_multimap <string, Student> dict);

unordered_multimap <string, Student> dict; // unordered_multimap: department name - student pairs. One key(dpart name) corresponds to multiple students.
// to get how many students in a department: dict.count(key), where key is the name of the department.
vector <string> dNameList; // department name list
vector <Department> dlist; // department list

/**
 * Get all students in this server and sort them in increasing order.
 * @param dict a map of department name - student pairs.
 * @return a vector of all the students in this server having been sorted in increasing order.
 */
vector <Student> getAllStusInOrder(unordered_multimap <string, Student> dict) {
    vector <Student> stus;
    for (auto it = dict.begin(); it != dict.end(); it++) {
        stus.push_back(it->second);
    }

    sort(stus.begin(), stus.end(), [](Student &stu1, Student &stu2) {
        return stu1.getId() < stu2.getId();
    });

    return stus;
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
 * Find the department name list based on the given name.
 * @param dnameList department name list
 * @param dname name of the department to look for
 * @return index of the department in the array
 */
int getDprtIndex(vector <string> dnameList, string dname) {
    for (int i = 0; i < dnameList.size(); i++) {
        if (dnameList[i] == dname) {
            return i;
        }
    }
    return -1;
}


/**
 * Find the student in the given department list, given the department name and the student's id.
 * @param dlist department list
 * @param dname department name
 * @param id student id
 * @return student in the list
 */
Student findStudent(vector <Department> dlist, string dname, int id) {
    Student result;

    for (int i = 0; i < dlist.size(); i++) {
        if (dlist[i].getDname() == dname) {
            vector <Student> tmps = dlist[i].getStudents();
            for (int i = 0; i < tmps.size(); i++) {
                if (tmps[i].getId() == id) {
                    return tmps[i];
                }
            }
        }
    }

    return result;
}


/**
 * Make department list given department name - student pairs.
 * @param dict department name - student pairs
 * @param dNameList department name list
 * @return department list
 */
vector <Department> makeDprtList(unordered_multimap <string, Student> dict, vector <string> dNameList) {
    vector <Department> dprtList; // list to return

    // collect all Student with the same key(aka dname) and store them in a Student vector
    for (int i = 0; i < dNameList.size(); i++) { // loop through each unique key in the dictionary
        vector <Student> stus; // students under the same department name in the given dictionary
        auto it = dict.begin();
        while (it != dict.end()) {
            if (it->first == dNameList[i]) {
                stus.push_back(it->second); // build student list for this department
            }
            it++;
        }

        Department dprt = Department(dNameList[i], stus);
        dprtList.push_back(dprt);
    }

    return dprtList;
}


// for debugging
void printVector(vector <string> v) {
    for (int i = 0; i < v.size(); i++) {
        cout << v[i] << " ";
    }
    cout << endl;
}


void printVector(vector<int> v) {
    for (int i = 0; i < v.size(); i++) {
        cout << v[i] << " ";
    }
    cout << endl;
}


/**
 * Calculate Cosine similarity of two arrays of scores
 * @param scores1
 * @param scores2
 * @return cosine similarity of the two arrays of scores
 */
float getCosSimilarity(vector<int> scores1, vector<int> scores2) {
    float dotProd, squareA, squareB = 0.0;

    for (int i = 0; i < scores1.size(); i++) {
        dotProd += scores1[i] * scores2[i] * 1.0;
        squareA += scores1[i] * scores1[i] * 1.0;
        squareB += scores2[i] * scores2[i] * 1.0;
    }

    float modA = sqrt(squareA);
    float modB = sqrt(squareB);

    return dotProd / (modA * modB);
}


/**
 * Find the most similar peer from the same department as the target student based on cosine similarity of their scores.
 * @param peers all the students in the same server
 * @param stu target student to find friend for
 * @return student id of a peer who is the most similar to the target student
 */
int findFriend(vector <Student> peers, Student stu) {
    if (peers.size() == 1) {
        return -1;
    }

    float max = INT_MIN;
    float tmp;
    int index;

    for (int i = 0; i < peers.size(); i++) {
        if (peers[i].getId() != stu.getId()) {
            tmp = getCosSimilarity(stu.getScores(), peers[i].getScores());
            if (tmp > max) {
                max = tmp;
                index = i;
            }
        }
    }

    return peers[index].getId();
}


/**
 * Given a student structure, return a string concatenated by Student GPA, Percentage Rank, Department GPA Mean, Department GPA Variance, Department Max GPA, Department Min GPA.
 * @param dprt department
 * @param stu student
 * @return string concatenated by Student GPA, Percentage Rank, Department GPA Mean, Department GPA Variance, Department Max GPA, Department Min GPA.
 */
string getDprtStuInfoString(Department dprt, Student stu, int friendId) {
    float gpaNum = stu.getGPA();
    std::stringstream stream1;
    stream1 << std::fixed << std::setprecision(1) << gpaNum;
    string gpa = stream1.str();

    float meanNum = dprt.getMean();
    std::stringstream stream2;
    stream2 << std::fixed << std::setprecision(1) << meanNum;
    string mean = stream2.str();

    float variNum = dprt.getVariance();
    std::stringstream stream3;
    stream3 << std::fixed << std::setprecision(1) << variNum;
    string variance = stream3.str();

    float maxNum = dprt.getMax();
    std::stringstream stream4;
    stream4 << std::fixed << std::setprecision(1) << maxNum;
    string max = stream4.str();

    float minNum = dprt.getMin();
    std::stringstream stream5;
    stream5 << std::fixed << std::setprecision(1) << minNum;
    string min = stream5.str();

    string rank = stu.getRank();

    string fid;
    if (friendId != -1) {
        std::stringstream stream6;
        stream6 << friendId;
        fid = stream6.str();
    } else {
        fid = "null";
    }

    return gpa + "," + rank + "," + mean + "," + variance + "," + max + "," + min + "," + fid;
}

/**
 * Get the unique keys of an unordered_multimap.
 * @param mp input map to get unique keys.
 * @return unique keys of the map.
 */
vector <string> getKeys(unordered_multimap <string, Student> mp) {
    vector <string> keys;
    for (unordered_multimap<string, Student>::iterator it = mp.begin(); it != mp.end(); it++) {
        keys.push_back(it->first);
    }

    set <string> s(keys.begin(), keys.end());
    vector <string> v(s.begin(), s.end());

    return v;
}

/**
 * Split the input string into an array of integers seperated by delimiter. Insert "-100" between commas where data is missing.
 * Adapted from Source @ https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
 * @param s input string to be split.
 * @param delimiter delimiter to split strings.
 * @return a vector of integers split by the delimiter.
 */
vector <string> splitInputLine(string s, string delimiter) {
    if (s.at(s.length() - 2) == ',') { // last (invisible) char of each line might be '\n' for .csv file
        s[s.length() - 1] = '-';
        s.append("100");
    }

    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector <string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        if (token == "") { //
            token = "-100";
        }
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));

    return res;
}


/**
 * Server A read the file and store the information.
 * @param filename file to read from.
 * @return a map of department names with their associated students.
 */
unordered_multimap <string, Student> storeInfo(string filename) {
    unordered_multimap <string, Student> result;
    fstream datafile;
    datafile.open(filename, ios::in);

    if (datafile.is_open()) {
        string input;
        string dName;
        int stuID;
        getline(datafile, input); // dump the first row (title row)
        while (getline(datafile, input)) {
            vector <string> row = splitInputLine(input, ","); //dName,stuID,score1,score2,score3...
            dName = row[0]; // department name
            stuID = stoi(row[1]); // student id
            vector <std::string> scorestr = {row.begin() + 2, row.end()};
            vector<int> scores;
            // convert scores vector from string to int
            std::transform(scorestr.begin(), scorestr.end(), std::back_inserter(scores),
                           [](const std::string &str) { return stoi(str); });

            Student stu = Student(stuID, scores);


            result.insert(std::make_pair(dName, stu));
        }
    }

    return result;
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

    // read the file dataA.csv and store the information in map structure "dict",
    // extract unique department names and store in dNameList for separate sending,
    // use department name - student pairs to generate a list of Departments
    dict = storeInfo("dataA.csv");
    dNameList = getKeys(dict);
    dlist = makeDprtList(dict, dNameList);


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

    // receive actions
    while (1) {
        // receive main server query
        memset(&buf, 0, sizeof buf);
        addr_len = sizeof their_addr;

        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *) &their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        // when received message starting with "!", send department name list to the main server
        if (buf[0] == '!') {
            string tmp = "A";
            // send department list to main server.
            for (size_t i = 0; i < dNameList.size(); i++) {
                tmp.append(",");
                tmp.append(dNameList[i]);
            }

            if ((numbytes_snd = sendto(sock_snd, tmp.c_str(), strlen(tmp.c_str()), 0, p_snd->ai_addr,
                                       p_snd->ai_addrlen)) == -1) {
                perror("serverA: sendto");
                exit(1);
            }

            // printf("This is what it has sent.%s", tmp.c_str());
            printf("Server A has sent a department list to Main Server\n\n");
        }

        // when receiving message starting with any char other than "!", parse message and prepare reply message
        if (buf[0] != '!') {
            string bufstr(buf);
            vector <string> request = split(bufstr, ",");
            string rdname = request[0]; // requested department name
            string ridstr = request[1]; // requested student id
            int rid = stoi(ridstr);
            printf("Server A has received a request for Student %d in %s\n", rid, rdname.c_str());

            // find the student in the given department:
            Student stu = findStudent(dlist, rdname, rid);

            // print message and send result thru UDP to main server
            string result = "A,";
            if (stu.getId() == INT_MAX) { // if not found
                printf("Student %d does not show up in %s\n", rid, rdname.c_str());

                // send not found message to main server.
                result += "/";

                if ((numbytes_snd = sendto(sock_snd, result.c_str(), strlen(result.c_str()), 0, p_snd->ai_addr,
                                           p_snd->ai_addrlen)) == -1) {
                    perror("serverA: sendto");
                    exit(1);
                }

                printf("Server A has sent \"Student %d not found\" to Main Server\n\n", rid);

            } else { // if found

                // get all the students in the same server and sort them in ascending order by id
                vector <Student> stus = getAllStusInOrder(dict);
                int friendId = findFriend(stus, stu);
                int index = getDprtIndex(dNameList, rdname);
                result += getDprtStuInfoString(dlist[index], stu, friendId);

                // send result to main server.
                if ((numbytes_snd = sendto(sock_snd, result.c_str(), strlen(result.c_str()), 0, p_snd->ai_addr,
                                           p_snd->ai_addrlen)) == -1) {
                    perror("serverA: sendto");
                    exit(1);
                }

                vector <string> stuInfo = split(result, ",");
                printf("Server A calculated following academic statistics for Student %d in %s:\n"
                       "Student GPA: %s\n"
                       "Percentage Rank: %s\n"
                       "Department GPA Mean: %s\n"
                       "Department GPA Variance: %s\n"
                       "Department Max GPA: %s\n"
                       "Department Min GPA: %s\n",
                       rid, rdname.c_str(), // student id, department name
                       stuInfo[1].c_str(), // gpa
                       stuInfo[2].c_str(), // rank
                       stuInfo[3].c_str(), // mean
                       stuInfo[4].c_str(), // variance
                       stuInfo[5].c_str(), // max gpa
                       stuInfo[6].c_str()); // min gpa

                printf("Server A has sent the result to Main Server\n\n");
            }

        }
    }

    close(sockfd);
    close(sock_snd);

    return 0;
}