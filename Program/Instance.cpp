//
// Created by Runqiu on 09/10/2023.
//

#include "Instance.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

Instance::Instance(int nbClient, const std::string &instNo, double proportion, bool expIsProportion) {
    std::string pathToInstance
        = "../Instances/" + std::to_string(nbClient) + "_" + instNo + "/";
    isProportion = expIsProportion;
    nbClients = nbClient;
    proportionRatio = proportion;

    readMatrixFromFile(pathToInstance);
    readStationInfoFromFile(pathToInstance, isProportion);
    readDissatisTable(pathToInstance);
    readPriorityTable(pathToInstance);
    readPriorityTableR(pathToInstance);
}

void Instance::readMatrixFromFile(const std::string &filepath) {
    std::ifstream inputFile(filepath + "time_matrix_" + std::to_string(nbClients) + ".txt");

    if (inputFile.is_open()) {
        std::string line;
        while (std::getline(inputFile, line)) {
            std::istringstream iss(line);
            std::vector<double> row;
            double temp;
            while (iss >> temp) { row.push_back(temp); }
            dist_mtx.push_back(row);
        }
    } else {
        throw std::runtime_error("Could not open file: " + filepath);
    }
}

void Instance::readStationInfoFromFile(const std::string &filepath, bool proportion) {
    networkInfo.reserve(nbClients + 1);
    // insert the depot at the beginning of the vector, with usableBike = inf,
    // brokenBike = 0, targetUsable = inf, capacity = inf
    networkInfo.emplace_back(0, INT_MAX, 0, INT_MAX, INT_MAX);
    std::ifstream inputFile(filepath + "station_info_" + std::to_string(nbClients) + ".txt");
    if (inputFile.is_open()) {
        std::string line;
        int id = 1;
        std::getline(inputFile, line); // skip the first line
        while (std::getline(inputFile, line)) {
            std::istringstream iss(line);
            // Read the rest of the numbers
            int real_id, usableBike, brokenBike, targetUsable, capacity;
            if (!proportion) {
                if (iss >> real_id >> capacity >> usableBike >> targetUsable >> brokenBike) {
                    networkInfo.emplace_back(id, usableBike, brokenBike, targetUsable, capacity);
                } else {
                    throw std::runtime_error("Could not read a number from line: " + line);
                }
            } else {
                if (iss >> real_id >> capacity >> usableBike >> targetUsable) {
                    if (usableBike > targetUsable) {
                        brokenBike = 0;
                    } else if (usableBike <= targetUsable) {
                        brokenBike = ceil(proportionRatio * (targetUsable - usableBike));
                        brokenBike = std::min(brokenBike, capacity - usableBike);
                    }
                    networkInfo.emplace_back(id, usableBike, brokenBike, targetUsable, capacity);
                } else {
                    throw std::runtime_error("Could not read a number from line: " + line);
                }
            }
            id++;
        }
    } else {
        throw std::runtime_error("Could not open file: " + filepath);
    }
}

void Instance::readDissatisTable(const std::string &filepath) {
    dissatTable.emplace_back();

    for (int i = 1; i <= nbClients; i++) {
        std::vector<std::vector<double>> data;
        std::string line;
        std::ifstream file(filepath + "dissat_table_" + std::to_string(i) + ".txt");
        if (!file) { std::cerr << "Could not open the file!\n"; }
        while (std::getline(file, line)) {
            // Create a string stream for the line
            std::stringstream ss(line);

            // Create a vector to hold the numbers on this line
            std::vector<double> row;

            // Read each number from the line
            double value;
            while (ss >> value) {
                // Add the number to the row vector
                row.push_back(value);
            }

            // Add the row to the main vector
            data.push_back(row);
        }
        dissatTable.push_back(data);
    }
}

void Instance::readPriorityTable(const std::string &filepath) {
    for (int i = 1; i <= nbClients; ++i) {
        std::vector<std::vector<double>> curPriorityTable;
        std::ifstream inFile(filepath + "BCRF_" + std::to_string(i) + ".txt");
        std::string line;
        while (std::getline(inFile, line)) {
            std::istringstream iss(line);
            std::vector<double> row;

            double value;
            while (iss >> value) { row.push_back(value); }

            curPriorityTable.push_back(row);
        }
        priorityTable.push_back(curPriorityTable);
    }
}

void Instance::readPriorityTableR(const std::string &filepath) {
    for (int i = 1; i <= nbClients; ++i) {
        std::vector<std::vector<double>> curPriorityTable;
        std::ifstream inFile(filepath + "BCRFR_" + std::to_string(i) + ".txt");
        std::string line;
        while (std::getline(inFile, line)) {
            std::istringstream iss(line);
            std::vector<double> row;

            double value;
            while (iss >> value) { row.push_back(value); }

            curPriorityTable.push_back(row);
        }
        priorityTableR.push_back(curPriorityTable);
    }
}
