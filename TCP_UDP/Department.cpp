//
// Created by Amy Lee on 4/17/23.
//

#include "Department.h"


Department::Department(std::string _dname, std::vector <Student> _students) {
    dname = _dname;

    std::sort(std::begin(_students), std::end(_students),
              [](Student &a, Student &b) -> bool {
                  return a.getGPA() > b.getGPA();
              });

    students = _students;
    int size = students.size();

    // set each student's rank
    // For example, if the student is rank 4 in 10 students, meaning the student is higher than 6 students in GPA, the percentage rank should be (10-4)/10=60%, meaning the student is higher than 60% of the students. We define that if only one student in the department, then the rank is (1-1)/1=0%.
    for (int i = 0; i < size; i++) {
        // i+1 : absolute rank of the student, should convert it into relative rank percentage.
        float rankNum = (size - i - 1) * 100.0 / size;
        rankNum = ((int) (rankNum * 100.0 + 0.5F)) / 100.0F;
        std::stringstream stream;
        stream << std::fixed << std::setprecision(1) << rankNum;
        std::string s = stream.str();
        students[i].setRank(s + "%"); // unit: xxx %, remember to round off to 1 decimal place.
    }

    calMean();
    calVariance();
}


void Department::calMean() {
    float sum = 0.0;
    for (int i = 0; i < students.size(); i++) {
        sum += students[i].getGPA();
    }
    gpaMean = sum / students.size();
}


void Department::calVariance() {
    float tmp = 0.0;
    for (int i = 0; i < students.size(); i++) {
        tmp += pow((students[i].getGPA() - gpaMean), 2);
    }
    gpaVariance = tmp / students.size();
}


float Department::getMin() {
    return students[students.size() - 1].getGPA();
}


float Department::getMax() {
    return students[0].getGPA();
}


float Department::getMean() {
    return gpaMean;
}


float Department::getVariance() {
    return gpaVariance;
}


std::string Department::getDname() {
    return dname;
}


std::vector<Student> Department::getStudents() {
    return students;
}