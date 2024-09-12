//
// Created by Transport02 on 24/11/2023.
//

#pragma once

#include "../Params.h"
#include "../RepositionScheme.h"
#include "../helpers/alias.h"
#include "Instance.h"
#include <memory>
#include <unordered_set>
#include <utility>


using RSchemeT = std::shared_ptr<RepositionSchemeTRK>;
using RSchemeR = std::shared_ptr<RepositionSchemeRPM>;
using RScheme = std::shared_ptr<RepositionSchemeBase>;

std::vector<RSchemeT> assignTruckScheme(Params &param, std::vector<int> &route, std::vector<int> &curUsable,
    std::vector<int> &curBroken, Instance &instance);
int adjustDepotLoading(Params &params, const int &demandDeviation, int curStation, std::vector<RSchemeT> &rSchemeVec,
    int prevDepot, double curStationPriority, double &maxOpTime,
    UnsatList &unSatLdStation);

std::vector<RSchemeR> assignRepairmanScheme(Params &params, std::vector<int> &route, std::vector<int> &curUsable,
    std::vector<int> &curBroken, Instance &instance);

std::pair<int, int> assignLoadingQuantities(std::vector<int> &curUsable, std::vector<int> &curBroken,
    std::vector<double> &maxOpTime, int &curStation, int &schemeIdx, int &truckUsable, int &truckBroken, Params &params,
    UnsatList &unSatLdStationU,
    UnsatList &unSatLdStationB, Instance &instance);
std::pair<int, int> assignUnloadingQuantities(std::vector<int> &curUsable, std::vector<int> &curBroken,
    std::vector<double> &maxOpTime, int &curStation, int &truckUsable, int &truckBroken, Params &params,
    std::vector<RSchemeT> &rSchemeVec, int &prevDepot, UnsatList &unSatLdStationU,
    UnsatList &unSatLdStationB,
    UnsatList &unSatUldStationU, std::vector<std::pair<int, double>> &uldExtraTime,
    Instance &instance);
std::pair<std::vector<double>, std::vector<double>> getMaxOpTime(Params &param, const std::vector<int> &curUsable,
    const std::vector<int> &curBroken, std::vector<int> &route, bool isRPM, Instance &instance);
double routeAdjustment(
    Params &param, std::vector<int> &route, std::vector<double> &priorityVector, bool isRPM, Instance &instance);
std::vector<RSchemeT> greedyAssignment(Params &param, std::vector<int> &route, std::vector<int> &curUsable,
    std::vector<int> &curBroken, std::vector<double> &maxOpTime, int &prevDepot, int &truckUsable, int &truckBroken,
    UnsatList &unsatLdStationU,
    UnsatList &unsatLdStationB,
    UnsatList &unSatUldStationU, std::vector<std::pair<int, double>> &idxExtraTime,
    Instance &instance);
double getPriority(Instance &instance, const int &station, const int &curUsable, const int &curBroken);
double getPriorityR(Instance &instance, const int &station, const int &curUsable, const int &curBroken);
void repairmanSchemeAmendment(Params &params, std::vector<RSchemeR> &rSchemeVec, std::vector<int> &curUsable,
    std::vector<int> &curBroken, std::unordered_set<int> &trkStationSet, Instance &instance);
void truckSchemeAmendment(Params &params, std::vector<RSchemeT> &rSchemeVec, std::vector<int> &curUsable,
    std::vector<int> &curBroken, Instance &instance);
void unSatUpdate(UnsatList &unSatStation, const int &curStation, const int &idx,
    const int &surplus, const double &priority);
void unSatUpdateOverwrite(UnsatList &unSatStation, const int &curStation,
    const int &idx, const int &surplus, const double &priority);
void extraUpdate(std::vector<std::pair<int, double>> &idxExtraTime, const int &curStation, const double &extraTime);
int loadBroken(const int &curStation, const int &truckUsable, const int &idx, std::vector<int> &curUsable,
    std::vector<int> &curBroken, std::vector<double> &maxOpTime, int &truckBroken, Params &params,
    UnsatList &unSatLdStationB, Instance &instance);

int unloadUsable(const int &curStation, const int &prevDepot,
    UnsatList &unSatLdStationU,
    UnsatList &unSatUldStationU, std::vector<std::pair<int, double>> &uldExtraTime,
    int stationResidualCapacity, std::vector<int> &curUsable, std::vector<int> &curBroken,
    std::vector<double> &maxOpTime, int &truckUsable, Params &params, std::vector<RSchemeT> &rSchemeVec,
    Instance &instance);

void suppLoadBroken(const std::vector<int> &curUsable, std::vector<int> &curBroken, std::vector<double> &maxOpTime,
    const int &curStation, const int &truckUsable, int &truckBroken, Params &params,
    const std::vector<RSchemeT> &rSchemeVec, UnsatList &unSatLdStationB,
    int &brokenLoad, Instance &instance);

int loadUsable(const int &curStation, const int &schemeIdx, const int &truckBroken,
    UnsatList &unSatLdStationU, std::vector<int> &curUsable,
    std::vector<int> &curBroken, std::vector<double> &maxOpTime, int &truckUsable, Params &params, Instance &instance);
void combineTheConsecutiveDuplicateNode(std::vector<std::vector<int>> &routes, bool isRPM);