//
// Created by Amy Lee on 4/17/23.
//

#include "Student.h"



Student::Student() {
    id = INT_MAX;
    std::vector<int> tmp;
    scores = tmp;
    rank = DUMMY ;
    gpa = DUMMY;
}

Student::Student(int _id, std::vector<int> _scores) { // if the student has no score in a course, set the score to -100.
    id = _id;
    scores = _scores;
    rank = DUMMY;
    gpa = DUMMY;
    calGPA();
}


void Student::calGPA() {
    int sum = 0;
    int cnt = 0;
    int tmp = 0;
    for (int i = 0; i < scores.size(); i++) {
        tmp = scores[i];
        if (tmp >= 0) {
            sum += tmp;
            cnt += 1;
        }
    }
    gpa = sum * 1.0 / cnt;
}


float Student::getGPA() {
    return gpa;
}


void Student::setRank(std::string _rank) {
    rank = _rank;
}


std::string Student::getRank() {
    return rank;
}


int Student::getId() {
    return id;
}


std::vector<int> Student::getScores() {
    return scores;
}