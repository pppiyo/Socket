//
// Created by Amy Lee on 4/17/23.
//

#ifndef CODE_STUDENT_H
#define CODE_STUDENT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <set>
#include <numeric>
#include <climits>

#define DUMMY -1.0;
#define DUMMYID -1;

class Student {
    int id;
    std::vector<int> scores;
    float gpa;
    std::string rank; // xx.x%
    void calGPA();

public:
    int getId();
    float getGPA();
    std::vector<int> getScores();
    std::string getRank();
    void setRank(std::string rank);
    Student(int _id, std::vector<int> _scores);
    Student();
//    bool cmpGPA(Student const & stu1, Student const & stu2);
};


#endif //CODE_STUDENT_H
