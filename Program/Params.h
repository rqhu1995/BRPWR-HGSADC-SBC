#pragma once

#include <chrono>
#include <climits>
#include <cmath>
#include <ctime>
#include <random>

struct AlgorithmParameters {
  int mu = 25;      // the minimum subpopulation size
  int lambda = 40;  // the number of offsprings generated in a generation
  double timeLimit; // Time limit for the algorithm
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
  int itEDU; // Number of iterations that an operator that successfully improved
             // a solution is repeated
  AlgorithmParameters ap;

  // Initialization from a given data set
  Params(int nbClients, int nbVeh, int nbRepairmen, int vehicleCapacity,
         int repairTime, int loadingTime, int mu, int lambda,
         int nbIterPenaltyManagement, double targetFeasible, int nbIterNoImp,
         double timeBudget, double penaltyCapacity, double timeLimit,
         int itEDU);
};
