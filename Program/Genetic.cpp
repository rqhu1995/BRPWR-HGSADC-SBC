#include "Genetic.h"
#include <chrono>
#include <fstream>
#include <iomanip>

bool terminateCondition(const Params &params, int nbIterNonProd) {
    return nbIterNonProd <= params.ap.nbIterNoImp
           && (params.ap.timeLimit == 0
               || std::chrono::duration_cast<std::chrono::seconds>(
                      std::chrono::high_resolution_clock::now() - params.startTime)
                          .count()
                      < params.ap.timeLimit);
}

Individual Genetic::run() {
    /* INITIAL POPULATION */
    std::cout << "----- BUILDING INITIAL POPULATION -----" << std::endl;
    population.generateInitialPopulation(params);
    while (population.feasibleSolution.size() == 0) { population.generateInitialPopulation(params); }
    std::cout << "----- BUILDING INITIAL POPULATION DONE! -----" << std::endl;
    population.updateAllBiasedFitness();
    // sort the fitness value of the feasible solutions in the ascending order
    std::sort(population.feasibleSolution.begin(), population.feasibleSolution.end(),
        [](const Individual &lhs, const Individual &rhs) { return lhs.eval.objVal < rhs.eval.objVal; });
    int nbIter;
    int nbIterNonProd = 0;
    double currentBestFitness = population.feasibleSolution[0].eval.objVal;
    Individual currentBestSolution = population.feasibleSolution[0];
    std::cout << "----- STARTING GENETIC ALGORITHM ----" << std::endl;
    std::cout << std::setw(20) << "ITERATION" << std::setw(20) << "NONIMP-ITER" << std::setw(20) << "BEST SOLUTION"
              << std::setw(20) << "TIME SPENT" << std::endl;
    for (nbIter = 0; terminateCondition(params, nbIterNonProd); nbIter++) {
        /* SELECTION AND CROSSOVER */
        Individual parentA = population.getBinaryTournament(params);
        Individual parentB = population.getBinaryTournament(params);
        Individual offspring = crossoverOX(parentA, parentB);
        bool improved = false;
        // population.addIndividualToSubpopulation(offspring, params);
        /* LOCAL SEARCH */
        localSearch.run(offspring);
        // add the offspring to the corresponding subpopulation if it is not in
        // the subpopulation, if offspring produces a better biased fitness,
        // update the current best fitness
        if (offspring.eval.isFeasible && offspring.eval.objVal < currentBestFitness) {
            currentBestFitness = offspring.eval.objVal;
            currentBestSolution = offspring;
            improved = true;
        }
        if (!offspring.eval.isFeasible && params.ran() % 2 == 0) // Repair half of the solutions in case of
                                                                 // infeasibility
        {
            localSearch.run(offspring);
            population.addIndividualToSubpopulation(offspring, params);
        }

        if (improved) { nbIterNonProd = 0; }

        // if nbIterNonProd is greater than the threshold, diversify the
        // population
        // if (nbIterNonProd % static_cast<int>(params.ap.nbIterNoImp * 0.4) == 0 && nbIterNonProd > 0) {
        if (nbIterNonProd % 1000 == 0 && nbIterNonProd > 0) {
            population.diversification(params);
            population.updateAllBiasedFitness();
        }

        /* DIVERSIFICATION, PENALTY MANAGEMENT AND TRACES */
        if (nbIter % params.ap.nbIterPenaltyManagement == 0 && nbIter > 0) {
            population.adjustmentBasedOnProportion(params);
            population.latestLoadOfFeasibleSolution.clear();
            population.latestLoadOfInfeasibleSolution.clear();
        }

        if (nbIter % 100 == 0 && nbIter > 0) {
            std::cout
                << std::setw(20) << nbIter << std::setw(20) << nbIterNonProd << std::setw(20) << currentBestFitness
                << std::setw(20)
                << std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - params.startTime).count()
                << std::endl;
        }

        if (!improved) { nbIterNonProd++; }
    }
    cpuTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - params.startTime).count();
    std::cout << "----- GENETIC ALGORITHM FINISHED AFTER " << nbIter << " ITERATIONS. TIME SPENT: " << std::fixed << std::setprecision(2) << cpuTime << std::endl;
    std::cout << "----- BEST SOLUTION FOUND: " << currentBestSolution.eval.objVal << std::endl;
    return currentBestSolution;
}

std::vector<int> Genetic::orderedCrossover(
    const std::vector<int> &routeA, const std::vector<int> &routeB, int routeType) {
    int minSize = std::min(routeA.size(), routeB.size());
    if (minSize <= 1) { return routeB; }

    // Guarantee t < y <= min(len(routeA), len(routeB))
    std::uniform_int_distribution<int> dist(0, minSize - 1);
    int t = dist(params.ran);
    int y;
    do { y = dist(params.ran); } while (y == t);
    // make sure t < y
    if (t > y) {
        int temp = t;
        t = y;
        y = temp;
    }
    std::vector<int> offspring(routeB.size());

    // Step 2: Copy the sub-tour from routeA to c1
    std::copy(routeA.begin() + static_cast<int>(t), routeA.begin() + static_cast<int>(y) + 1,
        offspring.begin() + static_cast<int>(t));
    if (routeType == 0) {
        // Step 3: Sweep routeB circularly from y + 1 to fill nodes in c1
        // circularly from y + 1. If the route type is 0, only the stations not
        // in c1 will be filled in.
        int indexB = y + 1;
        int indexC1 = y + 1;

        while (indexC1 != t) {
            if (indexB == routeB.size()) indexB = 0;
            if (indexC1 == offspring.size()) {
                indexC1 = 0;
                if (indexC1 == t) { break; }
            }

            if (std::find(offspring.begin(), offspring.end(), routeB[indexB]) == offspring.end()) {
                offspring[indexC1] = routeB[indexB];
                indexC1++;
            }
            indexB++;
        }
    } else if (routeType == 1) {
        // Step 3: Sweep routeB circularly from y + 1 to fill nodes in c1
        // circularly from y + 1. If the route type is 1, multiple visits are
        // allowed.
        int indexB = y + 1;
        int indexC1 = y + 1;

        while (indexC1 != t) {
            if (indexB == routeB.size()) indexB = 0;
            if (indexC1 == offspring.size()) {
                indexC1 = 0;
                if (indexC1 == t) { break; }
            }
            offspring[indexC1] = routeB[indexB];
            indexC1++;
            indexB++;
        }
    }
    return offspring;
}

/**
 * Do the ordered crossover between two parents
 * Specifically, each parent has a vector of the routes of the trucks and the
 * vector of the routes of the repairman. We first do the crossover between the
 * routes of the trucks, and then do the crossover between the routes of the
 * repairman. The routes taking part in the crossover are chosen share the same
 * index in the vector of the routes of the trucks and the vector of the routes
 * of the repairman. For example, the first route of the truck in parent 1 and
 * the first route of the truck in parent 2 are chosen to do the crossover, and
 * the first route of the repairman in parent 1 and the first route of the
 * repairman in parent 2 are chosen to do the crossover.
 */
Individual Genetic::crossoverOX(Individual &parent1, Individual &parent2) {
    std::vector<std::vector<int>> offspringChromTRK;
    std::vector<std::vector<int>> offspringChromRPM;
    // do the crossover between the routes of the trucks first
    for (int i = 0; i < params.nbVehicles; i++) {
        // pick up the routes of the trucks in parent 1 and parent 2
        std::vector<int> route1 = parent1.chromTRK[i];
        std::vector<int> route2 = parent2.chromTRK[i];
        std::vector<int> newRouteA;
        if (route1.size() >= 3 && route2.size() >= 3) {
            route1.erase(route1.begin());
            route1.erase(route1.end() - 1);
            route2.erase(route2.begin());
            route2.erase(route2.end() - 1);
            newRouteA = orderedCrossover(route1, route2, 1);
            // std::vector<int> newRouteB = orderedCrossover(route2, route1, 1);
            newRouteA.insert(newRouteA.begin(), 0);
            newRouteA.insert(newRouteA.end(), 0);
            // newRouteB.insert(newRouteB.begin(), 0);
            // newRouteB.insert(newRouteB.end(), 0);
        } else {
            newRouteA = route1;
        }
        offspringChromTRK.push_back(newRouteA);
        // offspringChromTRK.push_back(newRouteB);
    }
    // do the crossover between the routes of the repairman
    for (int i = 0; i < params.nbRepairmen; i++) {
        // pick up the routes of the repairman in parent 1 and parent 2
        std::vector<int> route1 = parent1.chromRPM[i];
        std::vector<int> route2 = parent2.chromRPM[i];
        // we exclude the initial and final depot in the crossover
        route1.erase(route1.begin());
        route1.erase(route1.end() - 1);
        route2.erase(route2.begin());
        route2.erase(route2.end() - 1);
        std::vector<int> newRouteA = orderedCrossover(route1, route2, 0);
        // std::vector<int> newRouteB = orderedCrossover(route2, route1, 0);
        // we add the initial and final depot back to the new route
        newRouteA.insert(newRouteA.begin(), 0);
        newRouteA.insert(newRouteA.end(), 0);
        // newRouteB.insert(newRouteB.begin(), 0);
        // newRouteB.insert(newRouteB.end(), 0);
        offspringChromRPM.push_back(newRouteA);
        // offspringChromRPM.push_back(newRouteB);
    }
    Individual offspring(params, instance, offspringChromRPM, offspringChromTRK);
    return offspring;
}

void Genetic::saveResults(Individual &bestSol, std::string fileName) {
    std::ofstream resultFile;
    resultFile.open(fileName, std::ios::app);
    resultFile << "the best solution is: " << std::endl;
    resultFile << "individual's fitness value: " << bestSol.eval.objVal << std::endl;
    resultFile << "individual's dissat value: " << bestSol.eval.dissat << std::endl;
    resultFile << "individual's emission value: " << bestSol.eval.emission << std::endl;
    resultFile << "individual's trkRoute: " << bestSol.eval.routeTRK << std::endl;
    resultFile << "individual's rpmRoute: " << bestSol.eval.routeRPM << std::endl;
    resultFile << "individual's trkOperationTime: " << bestSol.eval.operationTimeTRK << std::endl;
    resultFile << "individual's rpmOperationTime: " << bestSol.eval.operationTimeRPM << std::endl;
    resultFile << "the repositioning scheme for truck is: " << std::endl;
    for (auto &scheme : bestSol.repositionSchemeVectorTruck) {
        resultFile << "=======" << std::endl;
        for (auto &subscheme : scheme) {
            resultFile << subscheme->station << "\t";
            resultFile << "load " << subscheme->loadingQuantityU << " usable bikes;";
            resultFile << "load " << subscheme->loadingQuantityB << " broken bikes;";
            resultFile << "unload " << subscheme->unloadingQuantityU << " usable bikes;";
            resultFile << "unload " << subscheme->unloadingQuantityB << " broken bikes" << std::endl;
        }
    }
    resultFile << "the repositioning scheme for repairman is: " << std::endl;
    for (auto &scheme : bestSol.repositionSchemeVectorRepairman) {
        resultFile << "=======" << std::endl;
        for (auto &subscheme : scheme) {
            resultFile << subscheme->station << "\t";
            resultFile << "repair " << subscheme->repairingQuantity << " bikes" << std::endl;
        }
    }
    resultFile << std::endl;
    resultFile << "CPU time: " << cpuTime << std::endl;

    // print the user dissatisfaction of each station
    resultFile << "dissat at each station" << std::endl;
    for (int i = 0; i <= bestSol.eval.dissatisFaction.size() - 1; i++) {
        resultFile << "dissat[" << i+1 << "] = " << bestSol.eval.dissatisFaction[i] << std::endl;
    }
    resultFile.close();
}

Genetic::Genetic(Params &params, Instance &instance) :
    instance(instance), params(params), localSearch(params, instance), population(instance) {
}
