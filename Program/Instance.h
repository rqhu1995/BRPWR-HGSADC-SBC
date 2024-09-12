//
// Created by chkwon on 3/22/22.
//

#pragma once
#include "Station.h"
#include <climits>
#include <string>
#include <vector>

class Instance {
public:
    std::vector<std::vector<double>> dist_mtx;
    std::vector<std::vector<std::vector<double>>> dissatTable;
    std::vector<std::vector<std::vector<double>>> priorityTable;
    std::vector<std::vector<std::vector<double>>> priorityTableR;
    std::vector<double> service_time;
    std::vector<double> demands;
    std::vector<Station> networkInfo;
    int nbClients; // Number of clients (excluding the depot)
    double proportionRatio;
    bool isProportion = false;

    Instance(int nbClient, const std::string &instNo, double proportionRatio, bool proportion);
    void readMatrixFromFile(const std::string &filepath);
    void readStationInfoFromFile(const std::string &filepath, bool proportion);
    void readDissatisTable(const std::string &filepath);
    void readPriorityTable(const std::string &filepath);
    void readPriorityTableR(const std::string &filepath);
};
