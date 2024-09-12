#include "Individual.h"
#include "RepositionScheme.h"
#include "helpers/TimeHelper.h"
#include "helpers/Utils.h"
#include <algorithm>
#include <unordered_set>

Individual::Individual() = default;

/**
 * Constructor of the Individual class when there is no routes
 * @param params
 */
Individual::Individual(Params &params, Instance &instance) :
    chromRPM(std::vector<std::vector<int>>(params.nbRepairmen)),
    chromTRK(std::vector<std::vector<int>>(params.nbVehicles)) {
    // the initial inventory is assigned first
    std::vector<int> curUsable(params.nbClients + 1);
    std::vector<int> curBroken(params.nbClients + 1);

    for (int i = 1; i <= params.nbClients; i++) {
        curUsable[i] = instance.networkInfo[i].usableBike;
        curBroken[i] = instance.networkInfo[i].brokenBike;
    }
    // initialize the routes for repairmen and trucks
    rpmRouteInit(params, chromRPM, curUsable, curBroken, instance);
    trkRouteInit(params, chromTRK, curUsable, curBroken, instance);
    // initialize the individual
    buildIndividual(params, curUsable, curBroken, instance);
}

/**
 * Constructor of the Individual class when the routes are given
 * @param params
 * @param chromRPM
 * @param chromTRK
 */
Individual::Individual(Params &params, Instance &instance, const std::vector<std::vector<int>> &rpmRoutes,
    const std::vector<std::vector<int>> &trkRoutes) : chromRPM(rpmRoutes), chromTRK(trkRoutes) {
    // remove the intermediate 0 in chromRPM (i.e., remove 0 in chromRPM except
    // for the first and the end)
    for (auto &chrom : chromRPM) {
        if (chrom.size() > 2)
            chrom.erase(
                std::remove_if(chrom.begin() + 1, chrom.end() - 1, [](int i) { return i == 0; }), chrom.end() - 1);
    }

    std::vector<int> curUsable(params.nbClients + 1);
    std::vector<int> curBroken(params.nbClients + 1);
    buildIndividual(params, curUsable, curBroken, instance);
}

void Individual::buildIndividual(
    Params &params, std::vector<int> &curUsable, std::vector<int> &curBroken, Instance &instance) {
    combineTheConsecutiveDuplicateNode(chromRPM, true);
    combineTheConsecutiveDuplicateNode(chromTRK, false);
    for (int i = 1; i <= params.nbClients; i++) {
        curUsable[i] = instance.networkInfo[i].usableBike;
        curBroken[i] = instance.networkInfo[i].brokenBike;
    }

    greedyAssignmentForRepairman(params, curUsable, curBroken, chromRPM, instance);
    greedyAssignmentForTrucks(params, curUsable, curBroken, chromTRK, instance);

    // get the station id set appeared in truckSchemeVector
    std::unordered_set<int> rpmStationSet;
    // get the station id set appeared in truckSchemeVector
    for (auto &rpmScheme : repositionSchemeVectorRepairman) {
        for (auto &stationRepairScheme : rpmScheme) { rpmStationSet.insert(stationRepairScheme->station); }
    }

    for (int i = 0; i < params.nbVehicles; i++) {
        truckSchemeAmendment(params, repositionSchemeVectorTruck[i], curUsable, curBroken, instance);
    }

    for (int i = 0; i < params.nbRepairmen; i++) {
        repairmanSchemeAmendment(
            params, repositionSchemeVectorRepairman[i], curUsable, curBroken, rpmStationSet, instance);
    }

    // make sure each route in chromRPM and chromTRK starts from depot and ends at depot
    for (auto &chrom : chromRPM) {
        if (chrom[0] != 0) { chrom.insert(chrom.begin(), 0); }
        if (chrom[chrom.size() - 1] != 0) { chrom.push_back(0); }
    }

    for (auto &chrom : chromTRK) {
        if (chrom[0] != 0) { chrom.insert(chrom.begin(), 0); }
        if (chrom[chrom.size() - 1] != 0) { chrom.push_back(0); }
    }

    feasibilityCheckOfSolution(params, instance);
    solutionEvaluation(params);
}

void Individual::rpmRouteInit(Params &params, std::vector<std::vector<int>> &rpmVector, std::vector<int> &curUsable,
    std::vector<int> &curBroken, Instance &instance) {
    // the unVisited list is initialized as the set of all the stations from 1 to
    // nbClients
    std::unordered_set<int> unVisited = {};
    for (int i = 1; i <= params.nbClients; i++) { unVisited.insert(i); }
    for (int i = 0; i < params.nbRepairmen; i++) {
        rpmVector[i].push_back(0);
        int curStation = 0;
        double remainingTime = params.timeBudget;
        while (remainingTime > 0 && !unVisited.empty()) {
            // randomly pick up one station from the unvisited station
            std::unordered_set<int> stationCandidates = unVisited;
            int nextStation
                = *std::next(stationCandidates.begin(), static_cast<int>(params.ran() % stationCandidates.size()));
            double timeToNextStation = instance.dist_mtx[curStation][nextStation] * 1.68;
            double timeToDepot = instance.dist_mtx[nextStation][0] * 1.68;
            double timeLeftForRepair = remainingTime - timeToNextStation - timeToDepot;
            // if the next station is already visited, we need to randomly pick up
            // another station stationCandidates is initialized as the unvisited stations
            while (timeLeftForRepair < params.repairTime && !stationCandidates.empty()) {
                // randomly pick up one station from the stationCandidates
                nextStation
                    = *std::next(stationCandidates.begin(), static_cast<int>(params.ran() % stationCandidates.size()));
                stationCandidates.erase(nextStation);
                timeToNextStation = instance.dist_mtx[curStation][nextStation] * 1.68;
                timeToDepot = instance.dist_mtx[nextStation][0] * 1.68;
                timeLeftForRepair = remainingTime - timeToNextStation - timeToDepot;
            }
            // if the loop exit with stationCandidates not empty, we find a station to
            // repair
            if (!stationCandidates.empty()) {
                remainingTime -= timeToNextStation;
                remainingTime -= timeToDepot;
                int repairQuantity = std::min(
                    instance.networkInfo[nextStation].brokenBike, TimeHelper::maxRepairQByTime(params, remainingTime));
                remainingTime -= instance.dist_mtx[curStation][nextStation] + params.repairTime * repairQuantity;
                curBroken[nextStation] -= repairQuantity;
                curUsable[nextStation] += repairQuantity;
                // update the current station
                curStation = nextStation;
                rpmVector[i].push_back(nextStation);
                // update the visited list
                unVisited.erase(nextStation);
                remainingTime += timeToDepot;
            } else {
                rpmVector[i].push_back(0);
                break;
            }
        }
    }
}

void Individual::trkRouteInit(Params &params, std::vector<std::vector<int>> &trkVector, std::vector<int> &curUsable,
    std::vector<int> &curBroken, Instance &instance) {
    for (int i = 0; i < params.nbVehicles; i++) {
        int truckUsablePrev = 0;
        int truckBrokenPrev = 0;
        // the reachable list should be able to store the station obj, the gain/cost
        // ratio, and the number of bikes to repair
        std::vector<ReachableListCandidateTRK> reachableList;
        int curStation = 0;
        double remainingTime = params.timeBudget;
        while (remainingTime > 0) {
            double remainingTimeBak = remainingTime;
            int truckUsablePrevBak = truckUsablePrev;
            int truckBrokenPrevBak = truckBrokenPrev;
            for (int j = 1; j <= params.nbClients; j++) {
                if (j == curStation) { continue; }
                remainingTime -= instance.dist_mtx[curStation][j];
                remainingTime -= instance.dist_mtx[j][0];
                // check the status of the station
                if (curUsable[j] <= instance.networkInfo[j].targetUsable) {
                    int maxDeliveryQuantityU;
                    int maxCollectionQuantityB = 0;
                    if (remainingTime > 0) {
                        maxDeliveryQuantityU
                            = std::min(instance.networkInfo[j].targetUsable - curUsable[j], truckUsablePrev);
                        remainingTime -= maxDeliveryQuantityU * params.loadingTime;
                        truckUsablePrev -= maxDeliveryQuantityU;
                        if (remainingTime >= 2 * params.loadingTime) {
                            maxCollectionQuantityB = std::min(
                                std::min(curBroken[j], params.vehicleCapacity - truckUsablePrev - truckBrokenPrev),
                                static_cast<int>(std::floor(remainingTime / params.loadingTime / 2)));
                        }
                        if (maxDeliveryQuantityU + maxCollectionQuantityB > 0) {
                            ReachableListCandidateTRK candidate;
                            candidate.station = j;
                            candidate.deliveryQuantityU = maxDeliveryQuantityU;
                            candidate.collectionQuantityB = maxCollectionQuantityB;
                            reachableList.push_back(candidate);
                        }
                    }
                } else {
                    // the station is in surplus of bikes and needs to collect bikes
                    int maxCollectionQuantityU;
                    int maxCollectionQuantityB = 0;
                    if (remainingTime > 0) {
                        maxCollectionQuantityU
                            = std::min(std::min(curUsable[j] - instance.networkInfo[j].targetUsable,
                                           params.vehicleCapacity - truckUsablePrev - truckBrokenPrev),
                                static_cast<int>(std::floor(remainingTime / params.loadingTime / 2)));
                        remainingTime -= 2 * maxCollectionQuantityU * params.loadingTime;
                        truckUsablePrev += maxCollectionQuantityU;
                        if (remainingTime >= 2 * params.loadingTime) {
                            maxCollectionQuantityB = std::min(
                                std::min(curBroken[j], params.vehicleCapacity - truckUsablePrev - truckBrokenPrev),
                                static_cast<int>(std::floor(remainingTime / params.loadingTime / 2)));
                        }
                        if (maxCollectionQuantityU + maxCollectionQuantityB > 0) {
                            ReachableListCandidateTRK candidate;
                            candidate.station = j;
                            candidate.collectionQuantityU = maxCollectionQuantityU;
                            candidate.collectionQuantityB = maxCollectionQuantityB;
                            reachableList.push_back(candidate);
                        }
                    }
                }
                remainingTime = remainingTimeBak;
                truckUsablePrev = truckUsablePrevBak;
                truckBrokenPrev = truckBrokenPrevBak;
            }

            if (reachableList.empty()) {
                if (remainingTime > 0 && curStation != 0) {
                    remainingTime -= instance.dist_mtx[curStation][0];
                    truckBrokenPrev = 0;
                    truckUsablePrev = 0;
                    trkVector[i].push_back(0);
                    curStation = 0;
                } else {
                    break;
                }
            }

            // If the reachableList is empty, and we still have remaining time, we are
            // sure to go back to depot
            else {
                int selectedStationIdx;
                if (reachableList.size() > 2) {
                    selectedStationIdx = static_cast<int>(params.ran() % reachableList.size());
                } else {
                    selectedStationIdx = 0;
                }
                int selectedStation = reachableList[selectedStationIdx].station;
                int deliveryQuantityU = reachableList[selectedStationIdx].deliveryQuantityU;
                int collectionQuantityU = reachableList[selectedStationIdx].collectionQuantityU;
                int deliveryQuantityB = reachableList[selectedStationIdx].deliveryQuantityB;
                int collectionQuantityB = reachableList[selectedStationIdx].collectionQuantityB;
                // add the selected station to the route
                trkVector[i].push_back(selectedStation);
                remainingTime -= instance.dist_mtx[curStation][selectedStation]
                                 + params.loadingTime * (collectionQuantityU + collectionQuantityB);
                // update the inventories of usable and broken bikes
                curUsable[selectedStation] += deliveryQuantityU - collectionQuantityU;
                curBroken[selectedStation] += deliveryQuantityB - collectionQuantityB;
                truckBrokenPrev += collectionQuantityB - deliveryQuantityB;
                truckUsablePrev += collectionQuantityU - deliveryQuantityU;
                // update the current station
                curStation = selectedStation;
                reachableList.clear();
            }
        }
        // if trkVector is not started with depot, insert depot at the beginning,
        // and if trkVector is not ended with depot, insert depot at the end
        if (trkVector[i].empty()) {
            trkVector[i].push_back(0);
            trkVector[i].push_back(0);
        } else {
            if (trkVector[i][0] != 0) { trkVector[i].insert(trkVector[i].begin(), 0); }
            if (trkVector[i][trkVector[i].size() - 1] != 0) { trkVector[i].push_back(0); }
        }
    }
}

void Individual::greedyAssignmentForTrucks(Params &params, std::vector<int> &curUsable, std::vector<int> &curBroken,
    std::vector<std::vector<int>> &trkVector, Instance &instance) {
    std::vector<std::vector<RSchemeT>> repositionSchemeOverallVector;
    // backup the current inventory
    std::vector<int> curUsableBak = curUsable;
    std::vector<int> curBrokenBak = curBroken;
    for (int i = 0; i < params.nbVehicles; i++) {
        std::vector<RSchemeT> newScheme = assignTruckScheme(params, trkVector[i], curUsable, curBroken, instance);
        fixZeroLoading(curUsableBak, curBrokenBak, params, curUsable, curBroken, trkVector[i], instance, newScheme);
        repositionSchemeVectorTruck.push_back(newScheme);
    }
}
void Individual::fixZeroLoading(std::vector<int> &curUsableBak, std::vector<int> &curBrokenBak, Params &params,
    std::vector<int> &curUsable, std::vector<int> &curBroken, std::vector<int> &trkVector, Instance &instance,
    std::vector<RSchemeT> &newScheme) {
    // Use find_if to determine if there is any element with all quantities zero
    auto zeroLoadingIt = std::find_if(newScheme.begin() + 1, newScheme.end() - 1, [](const RSchemeT scheme) {
        return scheme->loadingQuantityU == 0 && scheme->loadingQuantityB == 0 &&
               scheme->unloadingQuantityU == 0 && scheme->unloadingQuantityB == 0;
    });

    if (zeroLoadingIt != newScheme.end() - 1) {
        // Use remove_if to filter out elements with all quantities zero and copy the rest to trkVector[i]
        trkVector = {0}; // Initialize with 0
        for (auto it = newScheme.begin() + 1; it != newScheme.end() - 1; ++it) {
            if (!((*it)->loadingQuantityU == 0 && (*it)->loadingQuantityB == 0 &&
                    (*it)->unloadingQuantityU == 0 && (*it)->unloadingQuantityB == 0)) {
                trkVector.push_back((*it)->station);
            }
        }
        trkVector.push_back(0);
            curUsable = curUsableBak;
            curBroken = curBrokenBak;
            newScheme = assignTruckScheme(params, trkVector, curUsable, curBroken, instance);
    }
}

void Individual::greedyAssignmentForRepairman(Params &params, std::vector<int> &curUsable, std::vector<int> &curBroken,
    std::vector<std::vector<int>> &rpmVector, Instance &instance) {
    std::vector<std::vector<RSchemeR>> repositionSchemeOverallVector;
    // backup the current inventory
    std::vector<int> curUsableBak = curUsable;
    std::vector<int> curBrokenBak = curBroken;
    for (int i = 0; i < params.nbRepairmen; i++) {
        std::vector<RSchemeR> newScheme = assignRepairmanScheme(params, rpmVector[i], curUsable, curBroken, instance);
        bool containsZeroRepair = false;
        rpmVector[i] = {0};
        for (int j = 1; j < newScheme.size() - 1; j++) {
            if (newScheme[j]->repairingQuantity == 0) {
                containsZeroRepair = true;
            } else {
                rpmVector[i].push_back(newScheme[j]->station);
            }
        }
        rpmVector[i].push_back(0);

        if (containsZeroRepair) {
            curUsable = curUsableBak;
            curBroken = curBrokenBak;
            newScheme = assignRepairmanScheme(params, rpmVector[i], curUsable, curBroken, instance);
        }

        repositionSchemeVectorRepairman.push_back(newScheme);
    }
}

void Individual::feasibilityCheckOfSolution(Params &params, Instance &instance) {
    this->eval.emission = 0;
    this->eval.routeTRK = 0;
    this->eval.operationTimeTRK = 0;
    this->eval.dissat = 0;
    this->eval.capacityExcess = 0;
    this->eval.routeRPM = 0;
    this->eval.operationTimeRPM = 0;
    this->eval.isFeasible = true;
    this->eval.dissatisFaction = {};
    std::vector<int> initialUsable = {};
    std::vector<int> initialBroken = {};
    for (int i = 0; i <= params.nbClients; i++) {
        initialUsable.push_back(instance.networkInfo[i].usableBike);
        initialBroken.push_back(instance.networkInfo[i].brokenBike);
    }
    // traverse each reposition scheme and calculate the arriving time at each
    // station, mark the arriving time at the corresponding field of the
    // reposition scheme
    std::vector<RScheme> combinedFlattenedScheme = {};
    // for truck
    for (auto &trkScheme : repositionSchemeVectorTruck) {
        double arrivingTime = 0;
        int prevStation = 0;
        trkScheme[0]->arrivingTime = 0;
        double emission = 0;
        double routeTimeTRK = 0;
        double operationTimeTRK
            = params.loadingTime * (trkScheme[0]->loadingQuantityU + trkScheme[0]->loadingQuantityB);

        arrivingTime += params.loadingTime * (trkScheme[0]->loadingQuantityU + trkScheme[0]->loadingQuantityB);
        std::vector<RSchemeT> updatedSchemeTRK;
        updatedSchemeTRK.push_back(trkScheme[0]);
        // displayAnIndividual();
        for (int j = 1; j < trkScheme.size(); j++) {
            int curStation = trkScheme[j]->station;
            int loadingU = trkScheme[j]->loadingQuantityU;
            int loadingB = trkScheme[j]->loadingQuantityB;
            int unloadingU = trkScheme[j]->unloadingQuantityU;
            int unloadingB = trkScheme[j]->unloadingQuantityB;
            arrivingTime += instance.dist_mtx[prevStation][curStation];
            trkScheme[j]->arrivingTime = arrivingTime;
            arrivingTime += params.loadingTime * (loadingU + loadingB + unloadingU + unloadingB);
            combinedFlattenedScheme.push_back(trkScheme[j]);
            emission += 2.61 * (0.252 + 0.0003 * (trkScheme[j - 1]->truckBQ + trkScheme[j - 1]->truckUQ))
                        * instance.dist_mtx[prevStation][curStation] / 60.0 * 0.42;
            routeTimeTRK += instance.dist_mtx[prevStation][curStation];
            operationTimeTRK += params.loadingTime * (loadingU + loadingB + unloadingU + unloadingB);
            prevStation = curStation;
            updatedSchemeTRK.push_back(trkScheme[j]);
        }
        this->eval.emission += emission;
        this->eval.routeTRK += routeTimeTRK;
        this->eval.operationTimeTRK += operationTimeTRK;
        trkScheme = updatedSchemeTRK;
    }
    // for repairman
    for (auto &rpmScheme : repositionSchemeVectorRepairman) {
        double arrivingTime = 0;
        int prevStation = 0;
        rpmScheme[0]->arrivingTime = 0;
        double routeTimeRPM = 0;
        double operationTimeRPM = 0.;
        std::vector<RSchemeR> updatedSchemeRPM;
        updatedSchemeRPM.push_back(rpmScheme[0]);
        for (int j = 1; j < rpmScheme.size(); j++) {
            arrivingTime += instance.dist_mtx[prevStation][rpmScheme[j]->station] * 1.68;
            rpmScheme[j]->arrivingTime = arrivingTime;
            double repairTime = params.repairTime * rpmScheme[j]->repairingQuantity;
            arrivingTime += repairTime;
            routeTimeRPM += instance.dist_mtx[prevStation][rpmScheme[j]->station] * 1.68;
            operationTimeRPM += repairTime;
            prevStation = rpmScheme[j]->station;
            combinedFlattenedScheme.push_back(rpmScheme[j]);
            updatedSchemeRPM.push_back(rpmScheme[j]);
        }
        this->eval.routeRPM += routeTimeRPM;
        this->eval.operationTimeRPM += operationTimeRPM;
        rpmScheme = updatedSchemeRPM;
    }
    // print the combinedFlattenedScheme

    // sort the combinedFlattenedScheme by the arriving time
    std::sort(combinedFlattenedScheme.begin(), combinedFlattenedScheme.end(),
        [](const RScheme &a, const RScheme &b) { return a->arrivingTime < b->arrivingTime; });
    // traverse the combinedFlattenedScheme and perform the
    // loading/unloading/repairing operation to check whether a station's:
    // 1. usable inv + broken inv > capacity
    // 2. usable inv < 0
    // 3. broken inv < 0
    // notice for scheme with usableLoad is -1, it is a repairman scheme, and we
    // only need to perform repair using the repairingQuantity; for other scheme,
    // we need to perform loading and unloading of usable and broken bikes
    for (auto &comScheme : combinedFlattenedScheme) {
        int curStation = comScheme->station;
        if (curStation != 0) {
            if (auto rpmPtr = dynamic_cast<RepositionSchemeRPM *>(comScheme.get())) {
                initialUsable[curStation] += rpmPtr->repairingQuantity;
                initialBroken[curStation] -= rpmPtr->repairingQuantity;
            } else {
                if (auto trkPtr = dynamic_cast<RepositionSchemeTRK *>(comScheme.get())) {
                    initialUsable[curStation] -= trkPtr->loadingQuantityU;
                    initialBroken[curStation] -= trkPtr->loadingQuantityB;
                    initialUsable[curStation] += trkPtr->unloadingQuantityU;
                    initialBroken[curStation] += trkPtr->unloadingQuantityB;
                }
            }
            if (initialUsable[curStation] < 0 || initialBroken[curStation] < 0
                || initialUsable[curStation] + initialBroken[curStation] > instance.networkInfo[curStation].capacity) {
                eval.isFeasible = false;
                eval.capacityExcess -= std::min(initialUsable[curStation], 0) + std::min(initialBroken[curStation], 0)
                                       + std::min(instance.networkInfo[curStation].capacity - initialUsable[curStation]
                                                      - initialBroken[curStation],
                                           0);
            }
        }
    }
    if (eval.isFeasible) {
        for (int i = 1; i <= params.nbClients; i++) {
            if (initialUsable[i] >= 0 && initialBroken[i] >= 0
                && initialUsable[i] + initialBroken[i] <= instance.networkInfo[i].capacity) {
                this->eval.dissat += instance.dissatTable[i][initialUsable[i]][initialBroken[i]];
                this->eval.dissatisFaction.push_back(instance.dissatTable[i][initialUsable[i]][initialBroken[i]]);
            }
        }
    }
}

void Individual::solutionEvaluation(Params &params) {
    this->eval.objVal = 2 * this->eval.dissat + 0.06 * this->eval.emission
                        + 1e-8
                              * (this->eval.routeTRK + this->eval.routeRPM + this->eval.operationTimeTRK
                                  + this->eval.operationTimeRPM)
                        + params.ap.penaltyCapacity * this->eval.capacityExcess;
}

void Individual::displayAnIndividual() {
    std::cout << "repositioning scheme for truck" << std::endl;
    for (auto &scheme : repositionSchemeVectorTruck) {
        std::cout << "=======" << std::endl;
        for (auto &subscheme : scheme) {
            std::cout << subscheme->station << "\t";
            std::cout << "load " << subscheme->loadingQuantityU << " usable bikes;";
            std::cout << "load " << subscheme->loadingQuantityB << " broken bikes;";
            std::cout << "unload " << subscheme->unloadingQuantityU << " usable bikes;";
            std::cout << "unload " << subscheme->unloadingQuantityB << " broken bikes" << std::endl;
        }
    }
    std::cout << "repositioning scheme for repairman" << std::endl;
    for (auto &scheme : repositionSchemeVectorRepairman) {
        std::cout << "=======" << std::endl;
        for (auto &subscheme : scheme) {
            std::cout << subscheme->station << "\t";
            std::cout << "repair " << subscheme->repairingQuantity << " bikes" << std::endl;
        }
    }
    std::cout << std::endl;

    std::cout << "individual's fitness value: " << eval.objVal << std::endl;
    std::cout << "individual's dissat value: " << eval.dissat << std::endl;
    std::cout << "individual's emission value: " << eval.emission << std::endl;
    std::cout << "individual's trkRoute: " << eval.routeTRK << std::endl;
    std::cout << "individual's rpmRoute: " << eval.routeRPM << std::endl;
    std::cout << "individual's trkOperationTime: " << eval.operationTimeTRK << std::endl;
    std::cout << "individual's rpmOperationTime: " << eval.operationTimeRPM << std::endl;

    std::cout << "dissat at each station" << std::endl;
    for (int i = 1; i <= eval.dissatisFaction.size(); i++) {
        std::cout << "dissat[" << i << "] = " << eval.dissatisFaction[i - 1] << std::endl;
    }
}
