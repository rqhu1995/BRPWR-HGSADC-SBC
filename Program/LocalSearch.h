#pragma once

#include "Individual.h"
#include <functional>

// Main local search structure
class LocalSearch {
public:
  Params &params;     // Problem parameters
  Instance &instance; // Problem instance
  // Constructor
  LocalSearch(Params &params, Instance &instance);
  void run(Individual &indiv); // Run the local search
  void swapTwoRandomElements(std::vector<std::vector<int>> &routes);

  void swapTwoSubtours(std::vector<std::vector<int>> &routes);

  void relocateOneNode(std::vector<std::vector<int>> &routes);

  void relocateOneSubtour(std::vector<std::vector<int>> &routes);

  void reverseOneSubtour(std::vector<std::vector<int>> &routes);

  void insertOneNode(std::vector<std::vector<int>> &routes, bool isRPM);

  void insertMultipleNodes(std::vector<std::vector<int>> &routes, bool isRPM);

  void deleteOneNode(std::vector<std::vector<int>> &routes);

  void deleteMultipleNodes(std::vector<std::vector<int>> &routes);

  Individual move_1(Individual &indiv);

  Individual move_2(Individual &indiv);

  Individual move_3(Individual &indiv);

  Individual move_4(Individual &indiv);

  Individual move_5(Individual &indiv);

  Individual move_6(Individual &indiv);

  Individual move_7(Individual &indiv);

  Individual move_8(Individual &indiv);

  Individual move_9(Individual &indiv);

  int getRand(int maxNum);

  void display2DVector(const std::vector<std::vector<int>> &routes);
  Individual
  executeAndDisplayMove(const std::function<Individual(Individual &)> &moveFunc,
                        Individual &indiv, const std::string &moveId);
};
