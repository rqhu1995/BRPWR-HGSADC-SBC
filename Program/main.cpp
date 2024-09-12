#include "Genetic.h"
#include "Individual.h"
#include "Instance.h"
#include "Params.h"
#include "helpers/Args.h"
#include "helpers/FileHelper.h"
#include "helpers/cmdparser.hpp"
#include <string>

int main(int argc, char *argv[]) {
  cli::Parser parser(argc, argv);
  Args::parseArgs(argc, argv);
  Instance instance(Args::nbStns, std::to_string(Args::nbInst),
                    Args::brokenProp, Args::isProportion);
  // Initialize the parameters
  Params params = Params(
      Args::nbStns, Args::nbTrk, Args::nbRpm, Args::vehicleCapacity,
      Args::repairTime, Args::loadingTime, Args::mu, Args::lambda,
      Args::nbIterPenaltyManagement, Args::targetFeasible, Args::nbIterNoImp,
      Args::timeBudget, Args::penaltyCapacity, Args::timeLimit, Args::itEDU);
  Genetic genetic(params, instance);
  Individual bestSol = genetic.run();
  FileHelper::saveResult(bestSol, params, Args::nbStns, Args::nbInst,
                         Args::brokenProp, genetic);
  //   std::vector<std::vector<int>> trkRoute = {{0, 1, 2, 13, 6, 0}};
  //   std::vector<std::vector<int>> rpmRoute = {{0, 9, 14, 12, 4, 15, 3, 0}};
  //   Individual bestSol = Individual(params, instance, rpmRoute, trkRoute);

  //   bestSol.displayAnIndividual();
  return 0;
}