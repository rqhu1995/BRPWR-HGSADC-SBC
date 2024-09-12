/*MIT License

Copyright(c) 2020 Thibaut Vidal

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#pragma once

#include "Instance.h"
#include "Params.h"
#include "helpers/Utils.h"
#include <cmath>

struct EvalIndiv {
    double capacityExcess = 0.; // Sum of excess load in all routes
    bool isFeasible = true;     // Feasibility status of the individual
    double dissat = 0.;         // Dissatisfaction of the solution
    double emission = 0.;       // Emission of the solution
    double objVal = 0.;         // Objective value of the solution
    double biasedFitness = 0.;  // Biased fitness value of an individual
    int similarity = 0;
    int rankSim = 0;
    int rankFit = 0;
    double routeTRK = 0.;
    double routeRPM = 0.;
    double operationTimeTRK = 0.;
    double operationTimeRPM = 0.;
    std::vector<double> dissatisFaction{0};
};

class Individual {
public:
    EvalIndiv eval;                         // Solution cost parameters
    std::vector<std::vector<int>> chromRPM; // For each repairman, the associated sequence of repairs
                                            // (complete solution)
    std::vector<std::vector<int>> chromTRK; // For each vehicle, the associated sequence of deliveries
                                            // (complete solution)
    std::vector<std::vector<RSchemeT>> repositionSchemeVectorTruck;
    std::vector<std::vector<RSchemeR>> repositionSchemeVectorRepairman;

    // Constructor of a random individual containing only a giant tour with a
    // shuffled visit order
    Individual();
    Individual(Params &params, Instance &instance);
    Individual(Params &params, Instance &instance, const std::vector<std::vector<int>> &rpmRoutes,
        const std::vector<std::vector<int>> &trkRoutes);
    void buildIndividual(Params &params, std::vector<int> &curUsable, std::vector<int> &curBroken, Instance &instance);
    static void trkRouteInit(Params &params, std::vector<std::vector<int>> &trkVector, std::vector<int> &curUsable,
        std::vector<int> &curBroken, Instance &instance);
    static void rpmRouteInit(Params &params, std::vector<std::vector<int>> &rpmVector, std::vector<int> &curUsable,
        std::vector<int> &curBroken, Instance &instance);
    void greedyAssignmentForTrucks(Params &params, std::vector<int> &curUsable, std::vector<int> &curBroken,
        std::vector<std::vector<int>> &trkVector, Instance &instance);
    void solutionEvaluation(Params &params);
    void greedyAssignmentForRepairman(Params &params, std::vector<int> &curUsable, std::vector<int> &curBroken,
        std::vector<std::vector<int>> &rpmVector, Instance &instance);
    void feasibilityCheckOfSolution(Params &params, Instance &instance);
    void displayAnIndividual();
    bool operator==(const Individual &other) const {
        // Two Individuals are considered equal if their 'chromRPM' and 'chromTRK'
        // members are equal.
        return (chromRPM == other.chromRPM && chromTRK == other.chromTRK);
    }
    void fixZeroLoading(std::vector<int> &curUsableBak, std::vector<int> &curBrokenBak, Params &params,
        std::vector<int> &curUsable, std::vector<int> &curBroken, std::vector<int> &trkVector, Instance &instance,
        std::vector<RSchemeT> &newScheme);
};
