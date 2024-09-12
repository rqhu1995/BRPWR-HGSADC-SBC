#include "Params.h"
#include <chrono>

Params::Params(int nbClients, int nbVeh, int nbRepairmen, int vehicleCapacity,
               int repairTime, int loadingTime, int mu, int lambda,
               int nbIterPenaltyManagement, double targetFeasible,
               int nbIterNoImp, double timeBudget, double penaltyCapacity,
               double timeLimit, int itEDU)
    : nbClients(nbClients), nbVehicles(nbVeh), nbRepairmen(nbRepairmen),
      timeBudget(timeBudget), repairTime(repairTime), loadingTime(loadingTime),
      vehicleCapacity(vehicleCapacity), itEDU(itEDU) {
  startTime = std::chrono::high_resolution_clock::now();
  ap = AlgorithmParameters(mu, lambda, timeLimit, nbIterPenaltyManagement,
                           targetFeasible, nbIterNoImp, penaltyCapacity);
  // Get the current time as the seed.
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  // Seed the generator with the current time.
  ran.seed(seed);
}
