#pragma once

#include "Individual.h"
#include "LocalSearch.h"
#include "Population.h"

class Genetic {
public:
  Params &params;          // Problem parameters
  Instance &instance;      // Instance of the problem
  LocalSearch localSearch; // Local Search structure
  Population population;   // Population (public for now to give access to the
                           // solutions, but should be improved later on)
  double cpuTime;          // CPU time

  // OX Crossover
  std::vector<int> orderedCrossover(const std::vector<int> &routeA,
                                    const std::vector<int> &routeB,
                                    int routeType);
  Individual crossoverOX(Individual &parent1, Individual &parent2);
  // Running the genetic algorithm until maxIterNonProd consecutive iterations
  // or a time limit
  Individual run();
  void saveResults(Individual &bestSol, std::string fileName);
  // Constructor
  Genetic(Params &params, Instance &instance);
};
