//
// Created by Runqiu on 9/10/2023.
//

#pragma once
#include <iostream>

class Station {
public:
    int id;
    int usableBike;
    int brokenBike;
    int targetUsable;
    int capacity;

    Station(int id, int usableBike, int brokenBike, int targetUsable, int capacity) :
        id(id), usableBike(usableBike), brokenBike(brokenBike), targetUsable(targetUsable), capacity(capacity) {
    }

    friend std::ostream &operator<<(std::ostream &os, const Station &station) {
        os << "Station " << station.id << ": " << station.usableBike << " " << station.brokenBike << " "
           << station.targetUsable << " " << station.capacity;
        return os;
    }
};
