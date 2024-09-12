//
// Created by Runqiu on 24/11/2023.
//

#include "Utils.h"
#include "Instance.h"
#include "RepInfo.h"
#include "SortHelper.h"
#include "TimeHelper.h"
#include <set>

double getPriority(Instance &instance, const int &station, const int &curUsable, const int &curBroken) {
    return instance.priorityTable[station - 1][curUsable][curBroken];
}

double getPriorityR(Instance &instance, const int &station, const int &curUsable, const int &curBroken) {
    return instance.priorityTableR[station - 1][curUsable][curBroken];
}

/**
 *
 * @param param
 * @param curUsable
 * @param curBroken
 * @param route
 * @param isRPM
 * @return
 */
std::pair<std::vector<double>, std::vector<double>> getMaxOpTime(Params &param, const std::vector<int> &curUsable,
    const std::vector<int> &curBroken, std::vector<int> &route, bool isRPM, Instance &instance) {
    double singleOperatingTime = isRPM ? param.repairTime : 2 * param.loadingTime;
    std::vector<double> priorityVector(instance.networkInfo.size() + 1, 0);
    for (size_t i = 1; i < route.size(); ++i) {
        if (!isRPM)
            priorityVector[route[i]]
                = (route[i] == 0) ? 0 : getPriority(instance, route[i], curUsable[route[i]], curBroken[route[i]]);
        else
            priorityVector[route[i]]
                = (route[i] == 0) ? 0 : getPriorityR(instance, route[i], curUsable[route[i]], curBroken[route[i]]);
    }
    std::vector<StationInfo> stationInfoVector;
    // make the route a set to remove the duplicate stations
    std::set<int> routeSet(route.begin(), route.end());
    for (auto &node : routeSet) {
        if (node != 0) {
            if (isRPM) {
                int repair = curBroken[node];
                StationInfo station(node, repair, priorityVector[node]);
                station.requiredTime = repair * param.repairTime;
                stationInfoVector.push_back(station);
            } else {
                int loadTaskU = std::max(curUsable[node] - instance.networkInfo[node].targetUsable, 0);
                int unloadTaskU = std::max(instance.networkInfo[node].targetUsable - curUsable[node], 0);
                int loadTaskB = curBroken[node];
                double priority = priorityVector[node];
                StationInfo station(node, loadTaskU, loadTaskB, unloadTaskU, priority);
                station.requiredTime = 2 * (loadTaskU + loadTaskB + unloadTaskU) * param.loadingTime;
                stationInfoVector.push_back(station);
            }
        }
    }

    double operatingTime = routeAdjustment(param, route, priorityVector, isRPM, instance);
    double prioritySum = std::accumulate(stationInfoVector.begin(), stationInfoVector.end(), 0.0,
        [](double sum, const StationInfo &station) { return sum + station.priority; });
    double extraTime = 0;
    for (auto &station : stationInfoVector) {
        station.maxOpTime = operatingTime * station.priority / prioritySum;
        if (station.maxOpTime < station.requiredTime) {
            station.insufficientTime = station.requiredTime - station.maxOpTime;
        } else {
            extraTime += station.maxOpTime - station.requiredTime;
            station.maxOpTime = station.requiredTime;
        }
    }

    Sort::sortBy(stationInfoVector, &StationInfo::priority, false);
    for (StationInfo &station : stationInfoVector) {
        if (extraTime <= singleOperatingTime) break;
        if (station.insufficientTime > 0) {
            double extraTimeToBeAdded = std::min(station.insufficientTime, extraTime);
            station.maxOpTime += extraTimeToBeAdded;
            extraTime -= extraTimeToBeAdded;
        }
    }
    std::vector<double> maxOperatingTime(instance.networkInfo.size(), 0);

    for (auto &station : stationInfoVector) { maxOperatingTime[station.stationID] = station.maxOpTime; }
    std::pair<std::vector<double>, std::vector<double>> maxOpPair = std::make_pair(maxOperatingTime, priorityVector);
    return maxOpPair;
}

double routeAdjustment(
    Params &param, std::vector<int> &route, std::vector<double> &priorityVector, bool isRPM, Instance &instance) {
    // Calculate boundary and factor based on flag
    double boundary = isRPM ? param.repairTime : 2 * param.loadingTime;
    double factor = isRPM ? 1.68 : 1.0;

    // Calculate operating time
    double operatingTime = param.timeBudget;
    for (size_t i = 1; i < route.size(); ++i) { operatingTime -= instance.dist_mtx[route[i - 1]][route[i]] * factor; }

    // Early exit if operating time is already above boundary
    if (operatingTime >= boundary) { return operatingTime; }

    // Pair station IDs with their priority, excluding the depot (0)
    std::vector<std::pair<int, double>> stationIDAndPriority;
    for (size_t i = 1; i < route.size(); ++i) {
        if (route[i] != 0) stationIDAndPriority.emplace_back(route[i], priorityVector[route[i]]);
    }

    // Sort stations by priority
    Sort::sortBy(stationIDAndPriority, &std::pair<int, double>::second, true);
    // Remove stations while operatingTime is below boundary
    for (const auto &station : stationIDAndPriority) {
        if (operatingTime < boundary) {
            auto it = std::find(route.rbegin(), route.rend(), station.first);
            if (it != route.rend()) {
                // Calculate the index of the station to be removed
                size_t index = std::distance(route.begin(), it.base()) - 1;

                // If we're not removing the first station, add back the cost from the
                // previous station to this one
                if (index > 0) { operatingTime += instance.dist_mtx[route[index - 1]][route[index]] * factor; }

                // If we're not removing the last station, add back the cost from this
                // station to the next one
                if (index < route.size() - 1) {
                    operatingTime += instance.dist_mtx[route[index]][route[index + 1]] * factor;
                }

                // If we're removing neither the first nor the last station, subtract
                // the cost of the direct path from the previous station to the next one
                if (index > 0 && index < route.size() - 1) {
                    operatingTime -= instance.dist_mtx[route[index - 1]][route[index + 1]] * factor;
                }

                route.erase(it.base() - 1); // Erase the station from the route
            }
        } else {
            break;
        } // Stop if we've reached the boundary condition
    }
    return operatingTime;
}

int adjustDepotLoading(Params &params, const int &demandDeviation, int curStation, std::vector<RSchemeT> &rSchemeVec,
    int prevDepot, double curStationPriority, double &maxOpTime, UnsatList &unSatUldStationU) {
    //  find the smallest residual capacity of the truck from
    //  repositionSchemeVector[prevDepot] to
    //  repositionSchemeVector[repositionSchemeVector.size() - 1]
    int minResidualCapacity = params.vehicleCapacity - rSchemeVec[prevDepot]->truckUQ - rSchemeVec[prevDepot]->truckBQ;

    for (int i = prevDepot + 1; i < rSchemeVec.size(); i++) {
        minResidualCapacity
            = std::min({minResidualCapacity, params.vehicleCapacity - rSchemeVec[i]->truckUQ - rSchemeVec[i]->truckBQ});
    }

    // calculate fd with the following strategies:
    // if prev depot loadingU > 0 and prev depot unloadingU == 0, fd =
    // repositionSchemeVector[prevDepot].loadingU if prev depot loadingU == 0 and
    // prev depot unloadingU > 0, fd =
    // -repositionSchemeVector[prevDepot].unloadingU if prev depot loadingU == 0
    // and prev depot unloadingU == 0, fd = 0 it's not possible for prev depot
    // loadingU > 0 and prev depot unloadingU > 0 because only one action is
    // allowed at each station
    int fd;

    if (rSchemeVec[prevDepot]->loadingQuantityU > 0 && rSchemeVec[prevDepot]->unloadingQuantityU == 0) {
        fd = rSchemeVec[prevDepot]->loadingQuantityU;
    } else if (rSchemeVec[prevDepot]->loadingQuantityU == 0 && rSchemeVec[prevDepot]->unloadingQuantityU > 0) {
        fd = -rSchemeVec[prevDepot]->unloadingQuantityU;
    } else {
        fd = 0;
    }

    // calculate ep as the minimum of the two:
    // 1. supplyDemand - usableUnload
    // 2. minResidualCapacity
    int epNoTime = std::min({demandDeviation, minResidualCapacity});
    int epByTime = TimeHelper::maxLoadQByTime(params, maxOpTime) - std::min(fd, 0);
    int vecSize = static_cast<int>(rSchemeVec.size());
    unSatUpdate(unSatUldStationU, curStation, vecSize, std::max(epNoTime - epByTime, 0), curStationPriority);
    int ep = std::min(epNoTime, epByTime);
    if (fd + ep > 0) {
        // last depot final loading decision is loading usable bikes
        rSchemeVec[prevDepot]->loadingQuantityU = fd + ep;
        rSchemeVec[prevDepot]->unloadingQuantityU = 0;
        maxOpTime -= 2 * (std::min(fd, 0) + ep) * params.loadingTime;
    } else {
        // last depot final loading decision is unloading usable bikes
        rSchemeVec[prevDepot]->loadingQuantityU = 0;
        rSchemeVec[prevDepot]->unloadingQuantityU = -fd - ep;
    }
    rSchemeVec[prevDepot]->truckUQ += ep;
    // update the truckUsable and truckBroken in the repoSchemeVector one by one
    // from prevDepot to the end
    for (int i = prevDepot + 1; i < rSchemeVec.size(); i++) {
        rSchemeVec[i]->truckUQ
            = rSchemeVec[i - 1]->truckUQ + rSchemeVec[i]->loadingQuantityU - rSchemeVec[i]->unloadingQuantityU;
        rSchemeVec[i]->truckBQ
            = rSchemeVec[i - 1]->truckBQ + rSchemeVec[i]->loadingQuantityB - rSchemeVec[i]->unloadingQuantityB;
    }
    return ep;
}

std::pair<int, int> assignUnloadingQuantities(std::vector<int> &curUsable, std::vector<int> &curBroken,
    std::vector<double> &maxOpTime, int &curStation, int &truckUsable, int &truckBroken, Params &params,
    std::vector<RSchemeT> &rSchemeVec, int &prevDepot, UnsatList &unSatLdStationU, UnsatList &unSatLdStationB,
    UnsatList &unSatUldStationU, std::vector<std::pair<int, double>> &uldExtraTime, Instance &instance) {
    int brokenLoad = 0;
    int stationResidualCapacity
        = instance.networkInfo[curStation].capacity - curUsable[curStation] - curBroken[curStation];
    if (stationResidualCapacity < instance.networkInfo[curStation].targetUsable - curUsable[curStation]) {
        int schemeSize = static_cast<int>(rSchemeVec.size());
        brokenLoad = loadBroken(curStation, truckUsable, schemeSize, curUsable, curBroken, maxOpTime, truckBroken,
            params, unSatLdStationB, instance);
        stationResidualCapacity
            = instance.networkInfo[curStation].capacity - curUsable[curStation] - curBroken[curStation];
    }
    int usableUnload = unloadUsable(curStation, prevDepot, unSatLdStationU, unSatUldStationU, uldExtraTime,
        stationResidualCapacity, curUsable, curBroken, maxOpTime, truckUsable, params, rSchemeVec, instance);

    if (maxOpTime[curStation] >= 2 * params.loadingTime) {
        suppLoadBroken(curUsable, curBroken, maxOpTime, curStation, truckUsable, truckBroken, params, rSchemeVec,
            unSatLdStationB, brokenLoad, instance);
    }

    return {usableUnload, brokenLoad};
}

void suppLoadBroken(const std::vector<int> &curUsable, std::vector<int> &curBroken, std::vector<double> &maxOpTime,
    const int &curStation, const int &truckUsable, int &truckBroken, Params &params,
    const std::vector<RSchemeT> &rSchemeVec, UnsatList &unSatLdStationB, int &brokenLoad, Instance &instance) {
    int vehicleResidualCapacity = params.vehicleCapacity - truckUsable - truckBroken;
    int newBrokenLoadNoTime = std::min({curBroken[curStation], vehicleResidualCapacity});
    int newBrokenLoadByTime = TimeHelper::maxLoadQByTime(params, maxOpTime[curStation]);
    int newBrokenLoad = std::min(newBrokenLoadNoTime, newBrokenLoadByTime);
    maxOpTime[curStation] -= 2 * newBrokenLoad * params.loadingTime;
    brokenLoad += newBrokenLoad;
    truckBroken += newBrokenLoad;
    curBroken[curStation] -= newBrokenLoad;
    unSatUpdateOverwrite(unSatLdStationB, curStation, rSchemeVec.size(),
        std::max(newBrokenLoadNoTime - newBrokenLoadByTime, 0),
        getPriority(instance, curStation, curUsable[curStation], curBroken[curStation]));
}

int unloadUsable(const int &curStation, const int &prevDepot, UnsatList &unSatLdStationU, UnsatList &unSatUldStationU,
    std::vector<std::pair<int, double>> &uldExtraTime, int stationResidualCapacity, std::vector<int> &curUsable,
    std::vector<int> &curBroken, std::vector<double> &maxOpTime, int &truckUsable, Params &params,
    std::vector<RSchemeT> &rSchemeVec, Instance &instance) {
    int usableUnload = 0;
    int supplyDemand
        = std::min({instance.networkInfo[curStation].targetUsable - curUsable[curStation], stationResidualCapacity});

    usableUnload = std::min({supplyDemand, truckUsable});

    curUsable[curStation] += usableUnload;
    extraUpdate(uldExtraTime, curStation, 2 * usableUnload * params.loadingTime);
    if (usableUnload == truckUsable && usableUnload != supplyDemand
        && maxOpTime[curStation] >= 2 * params.loadingTime) {
        // this means that the truck could not provide enough usable bikes to the
        // station, so we need to load more bikes at the previous nodes if possible.
        // We first check those non-depot previous visited loading nodes whose
        // surplus bikes were not fully loaded due to insufficient maxoptime, which
        // are stored in unSatLdStation.
        Sort::sortBy<3>(unSatLdStationU, false);
        while (usableUnload != supplyDemand && !unSatLdStationU.empty()
               && maxOpTime[curStation] >= 2 * params.loadingTime) {
            auto [station, idx, surplus, priority] = unSatLdStationU[0];

            int minimumResidual = params.vehicleCapacity - rSchemeVec[idx]->truckUQ - rSchemeVec[idx]->truckBQ;
            // find out the minimum residual capacity of the truck from
            // rSchemeVec[idx] to rSchemeVec[rSchemeVec.size() - 1]
            for (int i = idx + 1; i < rSchemeVec.size(); i++) {
                minimumResidual = std::min(
                    {minimumResidual, params.vehicleCapacity - rSchemeVec[i]->truckUQ - rSchemeVec[i]->truckBQ});
            }
            if (minimumResidual == 0) { break; }
            int addedLoadNoTime = std::min({supplyDemand - usableUnload, minimumResidual, surplus});
            int addedLoadByTime = TimeHelper::maxLoadQByTime(params, maxOpTime[curStation]);
            int addedLoad = std::min(addedLoadNoTime, addedLoadByTime);
            rSchemeVec[idx]->loadingQuantityU += addedLoad;
            curUsable[station] -= addedLoad;
            unSatUpdateOverwrite(unSatUldStationU, curStation, rSchemeVec.size(), addedLoadNoTime - addedLoad,
                getPriority(instance, curStation, curUsable[curStation], curBroken[curStation]));
            for (int i = idx; i < rSchemeVec.size(); i++) { rSchemeVec[i]->truckUQ += addedLoad; }
            curUsable[curStation] += addedLoad;
            usableUnload += addedLoad;
            surplus -= addedLoad;

            if (surplus == 0) {
                unSatLdStationU.erase(unSatLdStationU.begin());
            } else {
                std::get<2>(unSatLdStationU[0]) = surplus;
            }
            maxOpTime[curStation] -= 2 * addedLoad * params.loadingTime;
        }

        if (usableUnload != supplyDemand) {
            // if the usableUnload is still not enough, we need to load more bikes at
            // the depot
            int ep = adjustDepotLoading(params, supplyDemand - usableUnload, curStation, rSchemeVec, prevDepot,
                getPriority(instance, curStation, curUsable[curStation], curBroken[curStation]), maxOpTime[curStation],
                unSatUldStationU);
            usableUnload += ep;
            curUsable[curStation] += ep;
        }
    }
    truckUsable = rSchemeVec[rSchemeVec.size() - 1]->truckUQ - usableUnload;
    return usableUnload;
}

int loadBroken(const int &curStation, const int &truckUsable, const int &idx, std::vector<int> &curUsable,
    std::vector<int> &curBroken, std::vector<double> &maxOpTime, int &truckBroken, Params &params,
    UnsatList &unSatLdStationB, Instance &instance) {
    int brokenLoad = 0;

    if (maxOpTime[curStation] >= 2 * params.loadingTime) {
        int vehicleResidualCapacity = params.vehicleCapacity - truckUsable - truckBroken;
        int brokenLoadNoTime = std::min({curBroken[curStation], vehicleResidualCapacity});
        int brokenLoadByTime = TimeHelper::maxLoadQByTime(params, maxOpTime[curStation]);
        brokenLoad = std::min(brokenLoadNoTime, brokenLoadByTime);
        curBroken[curStation] -= brokenLoad;
        truckBroken += brokenLoad;
        maxOpTime[curStation] -= 2 * brokenLoad * params.loadingTime;
        unSatUpdateOverwrite(unSatLdStationB, curStation, idx, brokenLoadNoTime - brokenLoad,
            getPriority(instance, curStation, curUsable[curStation], curBroken[curStation]));
    }
    return brokenLoad;
}

std::pair<int, int> assignLoadingQuantities(std::vector<int> &curUsable, std::vector<int> &curBroken,
    std::vector<double> &maxOpTime, int &curStation, int &schemeIdx, int &truckUsable, int &truckBroken, Params &params,
    UnsatList &unSatLdStationU, UnsatList &unSatLdStationB, Instance &instance) {
    int usableLoad = 0;
    int brokenLoad = 0;
    // generate a random number between 0 and 1
    brokenLoad = loadBroken(curStation, truckUsable, schemeIdx, curUsable, curBroken, maxOpTime, truckBroken, params,
        unSatLdStationB, instance);
    usableLoad = loadUsable(curStation, schemeIdx, truckBroken, unSatLdStationU, curUsable, curBroken, maxOpTime,
        truckUsable, params, instance);

    return std::make_pair(usableLoad, brokenLoad);
}

int loadUsable(const int &curStation, const int &schemeIdx, const int &truckBroken, UnsatList &unSatLdStationU,
    std::vector<int> &curUsable, std::vector<int> &curBroken, std::vector<double> &maxOpTime, int &truckUsable,
    Params &params, Instance &instance) {
    int usableLoadNoTime = std::min({curUsable[curStation] - instance.networkInfo[curStation].targetUsable,
        params.vehicleCapacity - truckUsable - truckBroken});
    int usableLoadByTime = TimeHelper::maxLoadQByTime(params, maxOpTime[curStation]);
    int usableLoad = std::min(usableLoadNoTime, usableLoadByTime);

    maxOpTime[curStation] -= 2 * usableLoad * params.loadingTime;
    curUsable[curStation] -= usableLoad;
    truckUsable += usableLoad;
    unSatUpdateOverwrite(unSatLdStationU, curStation, schemeIdx, std::max(usableLoadNoTime - usableLoadByTime, 0),
        getPriority(instance, curStation, curUsable[curStation], curBroken[curStation]));
    return usableLoad;
}

std::vector<RSchemeT> assignTruckScheme(Params &param, std::vector<int> &route, std::vector<int> &curUsable,
    std::vector<int> &curBroken, Instance &instance) {
    std::vector<RSchemeT> rSchemeVec;
    auto [maxOpTime, priority] = getMaxOpTime(param, curUsable, curBroken, route, false, instance);
    int prevDepot = 0, truckUsable = 0, truckBroken = 0;
    UnsatList unSatLdStationU;
    UnsatList unSatLdStationB;
    UnsatList unSatUldStationU;
    std::vector<std::pair<int, double>> uldExtraTime;

    std::vector<int> curUsableBackup = curUsable;
    std::vector<int> curBrokenBackup = curBroken;
    std::vector<double> maxOpTimeBackup = maxOpTime;

    rSchemeVec = greedyAssignment(param, route, curUsable, curBroken, maxOpTime, prevDepot, truckUsable, truckBroken,
        unSatLdStationU, unSatLdStationB, unSatUldStationU, uldExtraTime, instance);

    maxOpTime = maxOpTimeBackup;
    // We subtract maxoptime for the items in uldExtraTime from the maxOpTime of
    // the stations in the route, sum up the extra time and store it in extraTime.
    // If extraTime > 0, we greedily give the extra time to the stations in
    // unSatLdStation following the priority order, then we do the loading and
    // unloading assignment again. If extraTime <= 0, we just return the
    // rSchemeVec.
    double extraTime = 0;

    for (auto &idx : uldExtraTime) {
        double modifiedTime = std::min({idx.second, maxOpTime[idx.first]});
        extraTime += modifiedTime;
        maxOpTime[idx.first] -= modifiedTime;
    }

    // combine the unSatLdStationU and unSatLdStationB and sort them by priority
    // (if some node appears in both unSatLdStationU and unSatLdStationB, we only
    // keep the priority of the one in unSatLdStationB)

    for (auto &idx : unSatUldStationU) {
        unSatUpdate(unSatLdStationU, std::get<0>(idx), std::get<1>(idx), std::get<2>(idx), std::get<3>(idx));
    }

    for (auto &idx : unSatLdStationB) {
        unSatUpdate(unSatLdStationU, std::get<0>(idx), std::get<1>(idx), std::get<2>(idx), std::get<3>(idx));
    }

    UnsatList unSatLdStation = unSatLdStationU;

    if (extraTime > 0) {
        Sort::sortBy<3>(unSatLdStation, false);
        while (extraTime > 0 && !unSatLdStation.empty()) {
            auto [station, idx, surplus, priority] = unSatLdStation[0];
            int adjustedLoadingTime = std::min({surplus * 2.0 * param.loadingTime, extraTime});
            maxOpTime[station] += adjustedLoadingTime;
            extraTime -= adjustedLoadingTime;
            unSatLdStation.erase(unSatLdStation.begin());
        }

        curUsable = curUsableBackup, curBroken = curBrokenBackup;
        prevDepot = 0, truckUsable = 0, truckBroken = 0;
        unSatLdStationU.clear();
        unSatLdStationB.clear();
        unSatUldStationU.clear();
        uldExtraTime.clear();
        rSchemeVec = greedyAssignment(param, route, curUsable, curBroken, maxOpTime, prevDepot, truckUsable,
            truckBroken, unSatLdStationU, unSatLdStationB, unSatUldStationU, uldExtraTime, instance);
    }

    return rSchemeVec;
}

std::vector<RSchemeR> assignRepairmanScheme(Params &params, std::vector<int> &route, std::vector<int> &curUsable,
    std::vector<int> &curBroken, Instance &instance) {
    // We set a maximum operating time for each element in the route based on the
    // priority of the stations (use proportional distribution)
    auto [maxOpTime, priority] = getMaxOpTime(params, curUsable, curBroken, route, true, instance);
    std::vector<RSchemeR> rSchemeVec;
    rSchemeVec.push_back(std::make_shared<RepositionSchemeRPM>());
    double extraTime = 0;
    for (int i = 1; i < route.size(); i++) {
        RSchemeR rScheme = std::make_shared<RepositionSchemeRPM>();
        int curStation = route[i];
        rScheme->station = curStation;
        if (curStation != 0) {
            int supplyDemand = std::max(instance.networkInfo[curStation].targetUsable - curUsable[curStation], 0);
            int repairQuantity = std::min(
                {supplyDemand, TimeHelper::maxRepairQByTime(params, maxOpTime[curStation]), curBroken[curStation]});
            maxOpTime[curStation] -= repairQuantity * params.repairTime;
            rScheme->repairingQuantity = repairQuantity;
            curUsable[curStation] += repairQuantity;
            curBroken[curStation] -= repairQuantity;
            extraTime += maxOpTime[curStation];
        }
        rSchemeVec.push_back(rScheme);
    }
    std::vector<std::pair<int, double>> routeByPriority;
    // create a vector of the route sorted by priority read from the priority
    // vector (priority[route[i]])
    for (int i = 0; i < route.size(); i++) {
        if (route[i] != 0)
            routeByPriority.emplace_back(
                route[i], getPriorityR(instance, route[i], curUsable[route[i]], curBroken[route[i]]));
    }
    Sort::sortBy(routeByPriority, &std::pair<int, double>::second, false);

    return rSchemeVec;
}

std::vector<RSchemeT> greedyAssignment(Params &param, std::vector<int> &route, std::vector<int> &curUsable,
    std::vector<int> &curBroken, std::vector<double> &maxOpTime, int &prevDepot, int &truckUsable, int &truckBroken,
    UnsatList &unsatLdStationU, UnsatList &unsatLdStationB, UnsatList &unSatUldStationU,
    std::vector<std::pair<int, double>> &idxExtraTime, Instance &instance) {
    std::vector<RSchemeT> rSchemeVec;
    int i = 0;
    for (auto &curStation : route) {
        RSchemeT rScheme = std::make_shared<RepositionSchemeTRK>();
        rScheme->station = curStation;
        if (curStation != 0) {
            if (curUsable[curStation] <= instance.networkInfo[curStation].targetUsable) {
                auto [usableUnload, brokenLoad] = assignUnloadingQuantities(curUsable, curBroken, maxOpTime, curStation,
                    truckUsable, truckBroken, param, rSchemeVec, prevDepot, unsatLdStationU, unsatLdStationB,
                    unSatUldStationU, idxExtraTime, instance);
                rScheme->unloadingQuantityU = usableUnload;
                rScheme->loadingQuantityB = brokenLoad;
            } else {
                auto [usableLoad, brokenLoad] = assignLoadingQuantities(curUsable, curBroken, maxOpTime, curStation, i,
                    truckUsable, truckBroken, param, unsatLdStationU, unsatLdStationB, instance);
                rScheme->loadingQuantityU = usableLoad;
                rScheme->loadingQuantityB = brokenLoad;
            }
        } else {
            rScheme->loadingQuantityU = 0;
            rScheme->loadingQuantityB = 0;
            rScheme->unloadingQuantityU = truckUsable;
            rScheme->unloadingQuantityB = truckBroken;
            prevDepot = i;
            truckUsable = 0;
            truckBroken = 0;
        }
        rScheme->truckUQ = truckUsable;
        rScheme->truckBQ = truckBroken;
        rSchemeVec.push_back(rScheme);
        i++;
    }

    return rSchemeVec;
}

void unSatUpdate(
    UnsatList &unSatStation, const int &curStation, const int &idx, const int &surplus, const double &priority) {
    if (surplus == 0) { return; }
    auto it = std::find_if(unSatStation.begin(), unSatStation.end(),
        [curStation](const auto &station) { return std::get<0>(station) == curStation; });
    if (it != unSatStation.end()) {
        std::get<2>(*it) += surplus;
        std::get<3>(*it) = priority;
    } else {
        unSatStation.emplace_back(curStation, idx, surplus, priority);
    }
}

void repairmanSchemeAmendment(Params &params, std::vector<RSchemeR> &rSchemeVec, std::vector<int> &curUsable,
    std::vector<int> &curBroken, std::unordered_set<int> &trkStationSet, Instance &instance) {
    // total time spent by the repairman
    double totalRouteAndRepairTime = 0;
    std::vector<std::pair<int, double>> stationAndPriority;
    for (int i = 1; i < rSchemeVec.size(); i++) {
        int cStd = rSchemeVec[i]->station;
        totalRouteAndRepairTime += instance.dist_mtx[rSchemeVec[i - 1]->station][cStd] * 1.68;
        totalRouteAndRepairTime += rSchemeVec[i]->repairingQuantity * params.repairTime;
        if (cStd != 0 && curBroken[cStd] > 0) {
            double priorityA = instance.dissatTable[cStd][curUsable[cStd]][curBroken[cStd]]
                               - instance.dissatTable[cStd][curUsable[cStd] + 1][curBroken[cStd] - 1];

            if (priorityA > 0) stationAndPriority.emplace_back(rSchemeVec[i]->station, priorityA);
        }
    }
    Sort::sortBy(stationAndPriority, &std::pair<int, double>::second, false);
    double extraTime = params.timeBudget - totalRouteAndRepairTime;
    while (extraTime > 0 && !stationAndPriority.empty()) {
        auto [curStation, priority] = stationAndPriority[0];
        auto it = std::find_if(rSchemeVec.begin() + 1, rSchemeVec.end(),
            [curStation](const auto &scheme) { return scheme->station == curStation; });
        if (it != rSchemeVec.end()) {
            int maxrepair = std::min(curBroken[curStation], TimeHelper::maxRepairQByTime(params, extraTime));
            double minDissatisfaction = instance.dissatTable[curStation][curUsable[curStation]][curBroken[curStation]];
            int actual = 0;
            for (int i = 0; i <= maxrepair; i++) {
                double curDissatisfaction
                    = instance.dissatTable[curStation][curUsable[curStation] + i][curBroken[curStation] - i];
                if (curDissatisfaction < minDissatisfaction) {
                    minDissatisfaction = curDissatisfaction;
                    actual = i;
                }
            }
            (*it)->repairingQuantity += actual;
            curUsable[curStation] += actual;
            curBroken[curStation] -= actual;
            extraTime -= actual * params.repairTime;
        }
        // remove stationAndPriority[0] from stationAndPriority
        stationAndPriority.erase(stationAndPriority.begin());
    }

    // while extraTime is still greater than 0, and we still have stations able to
    // repair but not in the route (curBroken > 0 and extraTime allows inserting
    // the station into the route and conduct at least one repair), we find the
    // station with the highest priority and insert it into the route, selecting
    // the insertion that adds the lowest travel time. If extraTime - that added
    // time > repairtime, we do the repair at the station, otherwise we give up
    // this station and try the next station with the highest priority.

    // firstly we gather the stations that are able to repair but not in the
    // route, with their priority
    std::vector<std::pair<int, double>> repairableStation;
    for (int i = 1; i < instance.networkInfo.size(); i++) {
        if (curBroken[i] > 0 && trkStationSet.find(i) == trkStationSet.end()) {
            // here priority is defined as the reduction in the dissatisfaction after
            // 1 bike repair, negative priority means that the station is not able to
            // repair, we set the priority to 0
            double priority = instance.dissatTable[i][curUsable[i]][curBroken[i]]
                              - instance.dissatTable[i][curUsable[i] + 1][curBroken[i] - 1];
            // std::cout << "~~~~station " << i << " has priority " << priority <<
            // std::endl;
            if (priority > 0) repairableStation.emplace_back(i, priority);
        }
    }
    Sort::sortBy(repairableStation, &std::pair<int, double>::second, false);

    while (extraTime > params.repairTime && !repairableStation.empty()) {
        auto [chosenStation, priority] = repairableStation[0];
        // std::cout << "inserting station " << chosenStation << std::endl;
        double minimumInsertionTime = (instance.dist_mtx[rSchemeVec[0]->station][chosenStation]
                                          + instance.dist_mtx[chosenStation][rSchemeVec[1]->station]
                                          - instance.dist_mtx[rSchemeVec[0]->station][rSchemeVec[1]->station])
                                      * 1.68;
        int insertIdx = 1;
        for (int i = 1; i < rSchemeVec.size(); i++) {
            double insertionTime = (instance.dist_mtx[rSchemeVec[i - 1]->station][chosenStation]
                                       + instance.dist_mtx[chosenStation][rSchemeVec[i]->station]
                                       - instance.dist_mtx[rSchemeVec[i - 1]->station][rSchemeVec[i]->station])
                                   * 1.68;
            if (insertionTime < minimumInsertionTime) {
                minimumInsertionTime = insertionTime;
                insertIdx = i;
            }
        }
        // std::cout << "insertion time " << minimumInsertionTime << std::endl;
        if (extraTime - minimumInsertionTime > params.repairTime) {
            // we insert the station into the route
            RSchemeR rScheme = std::make_shared<RepositionSchemeRPM>();
            rScheme->station = chosenStation;
            rScheme->repairingQuantity = 0;
            rSchemeVec.insert(rSchemeVec.begin() + insertIdx, rScheme);
            extraTime -= minimumInsertionTime;
            // we do the repair at the station
            int maxrepair = std::min(curBroken[chosenStation], TimeHelper::maxRepairQByTime(params, extraTime));
            // std::cout << "max repair at station " << chosenStation << " is " <<
            // maxrepair << std::endl; try repair from 0 to maxrepair, find the one
            // that generates the lowest user dissatisfaction at the station
            double minDissatisfaction
                = instance.dissatTable[chosenStation][curUsable[chosenStation]][curBroken[chosenStation]];
            int actual = 0;
            for (int i = 0; i <= maxrepair; i++) {
                double curDissatisfaction
                    = instance.dissatTable[chosenStation][curUsable[chosenStation] + i][curBroken[chosenStation] - i];
                if (curDissatisfaction < minDissatisfaction) {
                    minDissatisfaction = curDissatisfaction;
                    actual = i;
                }
            }
            rSchemeVec[insertIdx]->repairingQuantity += actual;
            curUsable[chosenStation] += actual;
            curBroken[chosenStation] -= actual;
            extraTime -= actual * params.repairTime;
        }
        repairableStation.erase(repairableStation.begin());
    }
}

// we try to amend the truck scheme if the time budget is not used up and allow
// for at least one bike loading and unloading specifically, we check the
// station which is now still in deficit, and find rhe minimum truck residual
// capacity from the station to the next closest depot in the scheme (including
// the residual capacity at the depot after the original depot operation), then
// we make the load at this station and unload them at the next depot if
// possible
void truckSchemeAmendment(Params &params, std::vector<RSchemeT> &rSchemeVec, std::vector<int> &curUsable,
    std::vector<int> &curBroken, Instance &instance) {
    // total time spent by the truck
    double totalRouteAndTruckTime = params.loadingTime
                                    * (rSchemeVec[0]->loadingQuantityU + rSchemeVec[0]->loadingQuantityB
                                        + rSchemeVec[0]->unloadingQuantityU + rSchemeVec[0]->unloadingQuantityB);
    std::vector<std::tuple<int, int, double>> stationAndPriority;
    for (int i = 1; i < rSchemeVec.size(); i++) {
        totalRouteAndTruckTime += instance.dist_mtx[rSchemeVec[i - 1]->station][rSchemeVec[i]->station];
        totalRouteAndTruckTime += params.loadingTime
                                  * (rSchemeVec[i]->loadingQuantityU + rSchemeVec[i]->loadingQuantityB
                                      + rSchemeVec[i]->unloadingQuantityU + rSchemeVec[i]->unloadingQuantityB);
        if (rSchemeVec[i]->station != 0
            && (curUsable[rSchemeVec[i]->station] != instance.networkInfo[rSchemeVec[i]->station].targetUsable
                || curBroken[rSchemeVec[i]->station] > 0)) {
            stationAndPriority.emplace_back(rSchemeVec[i]->station, i,
                getPriority(instance, rSchemeVec[i]->station, curUsable[rSchemeVec[i]->station],
                    curBroken[rSchemeVec[i]->station]));
        }
    }

    double extraTime = params.timeBudget - totalRouteAndTruckTime;
    // std::cout << extraTime << std::endl;
    while (extraTime >= 2 * params.loadingTime && !stationAndPriority.empty()) {
        auto [curStation, idx, priority] = stationAndPriority[0];
        int provider = std::max(curUsable[curStation] - instance.networkInfo[curStation].targetUsable, 0);
        if (provider > 0) {
            // find the next closest depot in the scheme (including the residual
            // capacity at the depot after the original depot operation
            int nextDepot = idx;
            int minimumResidual = params.vehicleCapacity - rSchemeVec[idx]->truckUQ - rSchemeVec[idx]->truckBQ;
            // find out the minimum residual capacity of the truck from
            // rSchemeVec[nextDepot] to rSchemeVec[rSchemeVec.size() - 1]
            for (int i = idx; i < rSchemeVec.size(); i++) {
                int deviation = std::max(
                    instance.networkInfo[rSchemeVec[i]->station].targetUsable - curUsable[rSchemeVec[i]->station], 0);
                if (rSchemeVec[i]->station == 0 || deviation > 0) {
                    nextDepot = i;

                    if (rSchemeVec[i]->station == 0) { deviation = 9999; }
                }
                minimumResidual = std::min(
                    {minimumResidual, params.vehicleCapacity - rSchemeVec[i]->truckUQ - rSchemeVec[i]->truckBQ});
                int addedLoad = std::min(
                    {deviation, minimumResidual, curUsable[curStation] - instance.networkInfo[curStation].targetUsable,
                        TimeHelper::maxLoadQByTime(params, extraTime)});
                minimumResidual -= addedLoad;
                rSchemeVec[idx]->loadingQuantityU += addedLoad;
                curUsable[curStation] -= addedLoad;
                rSchemeVec[nextDepot]->unloadingQuantityU += addedLoad;
                for (int i = idx; i < nextDepot; i++) { rSchemeVec[i]->truckUQ += addedLoad; }
                provider -= addedLoad;
                extraTime -= 2 * addedLoad * params.loadingTime;
            }
        }

        if (extraTime >= 2 * params.loadingTime && curBroken[curStation] > 0) {
            // find the next closest depot in the scheme (including the residual
            // capacity at the depot after the original depot operation
            int nextDepot = idx;
            for (int i = idx; i < rSchemeVec.size(); i++) {
                if (rSchemeVec[i]->station == 0) {
                    nextDepot = i;
                    break;
                }
            }
            int minimumResidual = params.vehicleCapacity - rSchemeVec[idx]->truckUQ - rSchemeVec[idx]->truckBQ;
            // find out the minimum residual capacity of the truck from
            // rSchemeVec[nextDepot] to rSchemeVec[rSchemeVec.size() - 1]
            for (int i = idx; i < nextDepot; i++) {
                minimumResidual = std::min(
                    {minimumResidual, params.vehicleCapacity - rSchemeVec[i]->truckUQ - rSchemeVec[i]->truckBQ});
            }
            int addedLoad
                = std::min({minimumResidual, curBroken[curStation], TimeHelper::maxLoadQByTime(params, extraTime)});
            rSchemeVec[idx]->loadingQuantityB += addedLoad;
            curBroken[curStation] -= addedLoad;
            rSchemeVec[nextDepot]->unloadingQuantityB += addedLoad;
            for (int i = idx; i < nextDepot; i++) { rSchemeVec[i]->truckBQ += addedLoad; }
            extraTime -= 2 * addedLoad * params.loadingTime;
        }
        stationAndPriority.erase(stationAndPriority.begin());
    }
}

void unSatUpdateOverwrite(
    UnsatList &unSatStation, const int &curStation, const int &idx, const int &surplus, const double &priority) {
    if (surplus == 0) { return; }
    auto it = std::find_if(unSatStation.begin(), unSatStation.end(),
        [curStation](const auto &station) { return std::get<0>(station) == curStation; });
    if (it != unSatStation.end()) {
        std::get<2>(*it) = surplus;
        std::get<3>(*it) = priority;
    } else {
        unSatStation.emplace_back(curStation, idx, surplus, priority);
    }
}

void extraUpdate(std::vector<std::pair<int, double>> &uldExtraTime, const int &curStation, const double &extraTime) {
    if (extraTime == 0) { return; }
    auto it = std::find_if(uldExtraTime.begin(), uldExtraTime.end(),
        [curStation](const auto &station) { return station.first == curStation; });
    if (it != uldExtraTime.end()) {
        std::get<1>(*it) = std::get<1>(*it) + extraTime;
    } else {
        uldExtraTime.emplace_back(curStation, extraTime);
    }
}

void combineTheConsecutiveDuplicateNode(std::vector<std::vector<int>> &routes, bool isRPM) {
    for (auto &route : routes) {
        if (route[0] == 0 && route[1] == 0 && route.size() == 2) { continue; }
        // Combine consecutive duplicate elements
        auto it = std::unique(route.begin(), route.end());
        route.resize(std::distance(route.begin(), it));
        // If not start with 0, add 0 to the beginning
        if (route.empty() || route[0] != 0) { route.insert(route.begin(), 0); }
        // If not end with 0, add 0 to the end
        if (route.empty() || route.back() != 0) { route.push_back(0); }
        if (isRPM && route.size() > 2) {
            auto first = route.begin() + 1; // Skip the first element
            auto last = route.end() - 1;    // Skip the last element

            route.erase(std::remove(first, last, 0),
                last); // Erase all zeros between first and last
        }
    }
}