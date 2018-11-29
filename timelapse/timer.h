#pragma once

#include <time.h>  

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

using namespace std;

double time_now() {
#ifdef _WIN32
    return double(clock())/CLOCKS_PER_SEC;
#else
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + double(tv.tv_usec)/1e6;
}
#endif

class Timer {
public:
    void add(string label, bool do_print=false) {
        labels.push_back(label);

        times.push_back(time_now());
        if (do_print) print();
    }

    void reset() {
        times.clear();
        labels.clear();
    }

    void print() {
        cout << toString() << endl;
    }

    string toString() {
        stringstream s;

        for (size_t i=1; i<times.size(); i++) {
            s 
                << labels[i] << ": " 
                << times[i]
                << "  ";
        }

        return s.str();
    }

    double toDouble(string s) {
        for (size_t i=0; i<labels.size(); i++) {
            if (labels[i] == s) {
                return times[i];
            }
        }

        fprintf(stderr, "Label not found: %s\n", s.c_str());
        return -1;
    }

    double diff(string label1, string label2) {
        return toDouble(label2) - toDouble(label1);
    }

protected:
    vector<double> times;
    vector<string> labels;
};

