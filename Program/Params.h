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

#include <chrono>
#include <climits>
#include <cmath>
#include <ctime>
#include <random>

struct AlgorithmParameters {
    int mu = 25;                       // the minimum subpopulation size
    int lambda = 40;                   // the number of offsprings generated in a generation
    double timeLimit;                  // Time limit for the algorithm
    int nbIterPenaltyManagement = 100; // Number of iterations before the penalty
    // coefficient is updated
    double targetFeasible = 0.2; // Target proportion of feasible solutions
    int nbIterNoImp = 5000;      // Number of iterations without improvement
    /* ADAPTIVE PENALTY COEFFICIENTS */
    double penaltyCapacity = 100.; // Penalty for one unit of capacity excess
    // (adapted through the search)
};

class Params {
public:
    /* START TIME OF THE ALGORITHM */
    std::chrono::time_point<std::chrono::system_clock> startTime;

    /* RANDOM NUMBER GENERATOR */
    std::mt19937 ran; // Using the fastest and simplest LCG. The quality of random
    // numbers is not critical for the LS, but speed is

    /* DATA OF THE PROBLEM INSTANCE */
    int nbClients;       // Number of clients (excluding the depot)
    int nbVehicles;      // Number of vehicles
    int nbRepairmen;     // Number of repairmen
    int vehicleCapacity; // Capacity limit
    int repairTime;      // Repair time for each bike
    double timeBudget;   // Time budget for the algorithm
    int loadingTime;
    int itEDU; // Number of iterations that an operator that successfully improved a solution is repeated
    AlgorithmParameters ap;

    // Initialization from a given data set
    Params(int nbClients, int nbVeh, int nbRepairmen, int vehicleCapacity, int repairTime, int loadingTime, int mu,
        int lambda, int nbIterPenaltyManagement, double targetFeasible, int nbIterNoImp, double timeBudget,
        double penaltyCapacity, double timeLimit, int itEDU);
};
