//
// Created by Amy Lee on 4/17/23.
//

#ifndef CODE_DEPARTMENT_H
#define CODE_DEPARTMENT_H

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
#include <cmath>
#include <iomanip>

#include "Student.h"


class Department {
    std::string dname; // department name
    std::vector <Student> students;
    float gpaMean;
    float gpaVariance;
    float gpaMax;
    float gpaMin;
    void calMean();
    void calVariance();

public:
    float getMean();
    float getVariance();
    float getMax();
    float getMin();
    std::string getDname();
    std::vector<Student> getStudents();
    Department(std::string _dname, std::vector <Student> _students);
};


#endif //CODE_DEPARTMENT_H
