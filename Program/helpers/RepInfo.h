#pragma once

struct StationInfo {
    int stationID = 0;
    int idxInRoute = -1;
    int leftLoadU = 0;
    int leftLoadB = 0;
    int leftUnloadU = 0;
    int repair = 0;
    double priority = 0.;
    double maxOpTime = 0.;
    double requiredTime = 0.;
    double insufficientTime = 0.;

    // constructor

    StationInfo(int stationID, int leftLoadU, int leftLoadB, int leftUnloadU, double priority) {
        this->stationID = stationID;
        this->leftLoadU = leftLoadU;
        this->leftLoadB = leftLoadB;
        this->leftUnloadU = leftUnloadU;
        this->priority = priority;
    }

    StationInfo(int stationID, int repair, double priority) {
        this->stationID = stationID;
        this->repair = repair;
        this->priority = priority;
    }

    StationInfo(int stationID, int idxInRoute, int leftLoadU, int leftLoadB, int leftUnloadU, double priority) {
        this->stationID = stationID;
        this->idxInRoute = idxInRoute;
        this->leftLoadU = leftLoadU;
        this->leftLoadB = leftLoadB;
        this->leftUnloadU = leftUnloadU;
        this->priority = priority;
    }

    StationInfo(int stationID, int idxInRoute, int repair, double priority) {
        this->stationID = stationID;
        this->idxInRoute = idxInRoute;
        this->repair = repair;
        this->priority = priority;
    }
};