#pragma once

#include "Individual.h"

class Population {
public:
  Instance &instance;
  std::vector<Individual> feasibleSolution;
  std::vector<Individual> infeasibleSolution;
  std::vector<Individual> latestLoadOfFeasibleSolution;
  std::vector<Individual> latestLoadOfInfeasibleSolution;
  double penaltyCoefficient = 100.0;
  Population(Instance &instance);
  void generateInitialPopulation(Params &params);
  void educateAndRepair(Individual &indiv, Params &params);
  void sizeControl(int subPopMaxSize, Params &params);
  double adjustmentBasedOnProportion(Params &params);
  void diversification(Params &params);
  Individual getBinaryTournament(Params &params);
  void updateAllBiasedFitness();
  void addIndividualToSubpopulation(Individual &indiv, Params &params);
};

void getSimilarity(std::vector<Individual> &subpopulation);
void getBiasedFitness(std::vector<Individual> &subpopulation);
