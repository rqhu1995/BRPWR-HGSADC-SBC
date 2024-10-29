#include "LocalSearch.h"
#include <functional>
#include <unordered_set>

// Constructor
LocalSearch::LocalSearch(Params &params, Instance &instance)
    : params(params), instance(instance) {}

int LocalSearch::getRand(int maxNum) {
  std::uniform_int_distribution<int> dist(0, maxNum - 1);
  return dist(params.ran);
}

// Run the local search
void LocalSearch::run(Individual &indiv) {
  using EduOp = std::function<Individual(Individual &)>;
  std::vector<std::pair<EduOp, std::string>> move_list = {
      std::make_pair([this](Individual &indiv) { return this->move_1(indiv); },
                     "move_1"),
      std::make_pair([this](Individual &indiv) { return this->move_2(indiv); },
                     "move_2"),
      std::make_pair([this](Individual &indiv) { return this->move_3(indiv); },
                     "move_3"),
      std::make_pair([this](Individual &indiv) { return this->move_4(indiv); },
                     "move_4"),
      std::make_pair([this](Individual &indiv) { return this->move_5(indiv); },
                     "move_5"),
      std::make_pair([this](Individual &indiv) { return this->move_6(indiv); },
                     "move_6"),
      std::make_pair([this](Individual &indiv) { return this->move_7(indiv); },
                     "move_7"),
      std::make_pair([this](Individual &indiv) { return this->move_8(indiv); },
                     "move_8"),
      std::make_pair([this](Individual &indiv) { return this->move_9(indiv); },
                     "move_9")};
  while (!move_list.empty()) {
    int index = getRand(move_list.size()); // choose a random index
    EduOp edu_op = move_list[index].first; // get the function at that index
    auto fit_v = indiv.eval.objVal;
    Individual educated_ind =
        executeAndDisplayMove(edu_op, indiv, move_list[index].second);
    move_list.erase(move_list.begin() + index); // remove the chosen function
    if (educated_ind.eval.objVal < fit_v) {
      int consecutive_imp = 0;
      int consecutive = 0;
      indiv = educated_ind;
      fit_v = indiv.eval.objVal;
      while (consecutive_imp < params.itEDU && consecutive < 1000) {
        executeAndDisplayMove(edu_op, educated_ind, move_list[index].second);
        if (educated_ind.eval.objVal >= fit_v) {
          consecutive_imp += 1;
        } else {
          consecutive_imp = 0;
          indiv = educated_ind;
          fit_v = indiv.eval.objVal;
        }
        consecutive += 1;
      }
      break;
    }
  }
}

// single node swap: randomly select one station in each route other than the
// first and last station, swap them
void LocalSearch::swapTwoRandomElements(std::vector<std::vector<int>> &routes) {
  for (auto &route : routes) {
    if (route.size() > 3) {
      int index1 =
          getRand(route.size() - 2) + 1; // Adjusted to exclude depot nodes
      int index2 =
          getRand(route.size() - 2) + 1; // Adjusted to exclude depot nodes
      while (index1 == index2) {
        index2 =
            getRand(route.size() - 2) + 1; // Adjusted to exclude depot nodes
      }
      std::swap(route[index1], route[index2]);
    }
  }
}

Individual LocalSearch::move_1(Individual &indiv) {
  // Extract chromRPM and chromTRK from indiv
  std::vector<std::vector<int>> chromRPM = indiv.chromRPM;
  std::vector<std::vector<int>> chromTRK = indiv.chromTRK;

  // Swap two random elements in chromRPM and chromTRK
  swapTwoRandomElements(chromRPM);
  swapTwoRandomElements(chromTRK);
  // Construct a new Individual with the modified chromRPM and chromTRK
  return {params, this->instance, chromRPM, chromTRK};
}

// subtour swap: pick up two subtours in each route, swap them. Before choosing
// the subtours, we need to make sure that the route has more than 3 nodes. We
// ignore the first and last nodes. We choose four indices sorted in ascending
// order: index1, index2, index3, index4. We swap the subtours [index1, index2]
// and [index3, index4]. Note index1 <= index2 < index3 <= index4. If index1 ==
// index2 or index3 == index4, the subtour is the node itself at that index.
void LocalSearch::swapTwoSubtours(std::vector<std::vector<int>> &routes) {
  for (auto &route : routes) {
    if (route.size() >
        4) { // Ensure there are more than 3 nodes excluding depot nodes
      int index1 =
          getRand(route.size() - 2) + 1; // Adjusted to exclude depot nodes
      int index2 =
          getRand(route.size() - 2) + 1; // Adjusted to exclude depot nodes
      int index3 =
          getRand(route.size() - 2) + 1; // Adjusted to exclude depot nodes
      int index4 =
          getRand(route.size() - 2) + 1; // Adjusted to exclude depot nodes

      // Sort the indices in ascending order
      std::vector<int> indices = {index1, index2, index3, index4};
      std::sort(indices.begin(), indices.end());

      // Ensure that index2 is not equal to index3 (to avoid overlapping
      // or invalid swap)
      while (indices[1] == indices[2]) {
        indices[1] = getRand(route.size() - 2) + 1; // Adjust index2
        std::sort(indices.begin(), indices.end());
      }

      // Clear the newRoute and fill it with the swapped subtours
      std::vector<int> newRoute;

      // Insert part before the first subtour
      newRoute.insert(newRoute.end(), route.begin(),
                      route.begin() + indices[0]);

      // Insert the second subtour
      newRoute.insert(newRoute.end(), route.begin() + indices[2],
                      route.begin() + indices[3] + 1);

      // Insert the part between the two subtours
      newRoute.insert(newRoute.end(), route.begin() + indices[1] + 1,
                      route.begin() + indices[2]);

      // Insert the first subtour
      newRoute.insert(newRoute.end(), route.begin() + indices[0],
                      route.begin() + indices[1] + 1);

      // Insert the part after the second subtour
      newRoute.insert(newRoute.end(), route.begin() + indices[3] + 1,
                      route.end());

      // Assign the new route back to the original route
      route = newRoute;
    }
  }
}

Individual LocalSearch::move_2(Individual &indiv) {
  // Extract chromRPM and chromTRK from indiv
  std::vector<std::vector<int>> chromRPM = indiv.chromRPM;
  std::vector<std::vector<int>> chromTRK = indiv.chromTRK;
  // Swap two subtours in chromRPM and chromTRK
  swapTwoSubtours(chromRPM);
  swapTwoSubtours(chromTRK);
  // Construct a new Individual with the modified chromRPM and chromTRK
  return {params, instance, chromRPM, chromTRK};
}

// single node relocation: choose one station in each route other than the first
// and last station, relocate it to another position
void LocalSearch::relocateOneNode(std::vector<std::vector<int>> &routes) {
  for (auto &route : routes) {
    if (route.size() > 3) {
      int index1 =
          getRand(route.size() - 2) + 1; // Adjusted to exclude depot nodes
      int node = route[index1];
      route.erase(route.begin() + index1);

      int index2;
      if (route.size() == 2) {
        index2 = 1; // Only one valid position to insert
      } else {
        do {
          index2 = getRand(route.size() - 1) + 1;
        } while (index2 == index1 && route.size() > 3);
      }

      route.insert(route.begin() + index2, node);
    }
  }
}

Individual LocalSearch::move_3(Individual &indiv) {
  // Extract chromRPM and chromTRK from indiv
  std::vector<std::vector<int>> chromRPM = indiv.chromRPM;
  std::vector<std::vector<int>> chromTRK = indiv.chromTRK;
  // Relocate one node in chromRPM and chromTRK
  relocateOneNode(chromRPM);
  relocateOneNode(chromTRK);
  // Construct a new Individual with the modified chromRPM and chromTRK
  return {params, instance, chromRPM, chromTRK};
}

// subtour relocation: choose one subtour in each route, relocate it to another
// position
void LocalSearch::relocateOneSubtour(std::vector<std::vector<int>> &routes) {
  for (auto &route : routes) {
    if (route.size() > 4) // Adjusted to account for depot nodes
    {
      int index1 =
          getRand(route.size() - 3) + 1; // Adjusted to exclude depot nodes
      int index2 =
          getRand(route.size() - 3) + 1; // Adjusted to exclude depot nodes
      while (index1 == index2) {
        index2 =
            getRand(route.size() - 3) + 1; // Adjusted to exclude depot nodes
      }
      if (index1 > index2) {
        std::swap(index1, index2);
      }
      std::vector<int> subroute(route.begin() + index1,
                                route.begin() + index2 + 1);
      route.erase(route.begin() + index1, route.begin() + index2 + 1);

      int index3;
      if (route.size() > 2) { // Adjusted to account for depot nodes
        index3 =
            getRand(route.size() - 2) + 1; // Adjusted to exclude depot nodes
      } else {
        index3 = 1; // Only one position left to insert, between the
                    // depot nodes
      }

      route.insert(route.begin() + index3, subroute.begin(), subroute.end());
    }
  }
}

Individual LocalSearch::move_4(Individual &indiv) {
  // Extract chromRPM and chromTRK from indiv
  std::vector<std::vector<int>> chromRPM = indiv.chromRPM;
  std::vector<std::vector<int>> chromTRK = indiv.chromTRK;
  // Relocate one subtour in chromRPM and chromTRK
  relocateOneSubtour(chromRPM);
  relocateOneSubtour(chromTRK);
  // Construct a new Individual with the modified chromRPM and chromTRK
  return {params, instance, chromRPM, chromTRK};
}

// 2-opt: choose one subtour in each route, reverse it
void LocalSearch::reverseOneSubtour(std::vector<std::vector<int>> &routes) {
  for (auto &route : routes) {
    if (route.size() > 3) {
      int index1 =
          getRand(route.size() - 2) + 1; // Adjusted to exclude depot nodes
      int index2 =
          getRand(route.size() - 2) + 1; // Adjusted to exclude depot nodes
      while (index1 == index2) {
        index2 =
            getRand(route.size() - 2) + 1; // Adjusted to exclude depot nodes
      }
      if (index1 > index2) {
        std::swap(index1, index2);
      }
      std::reverse(route.begin() + index1, route.begin() + index2 + 1);
    }
  }
}

Individual LocalSearch::move_5(Individual &indiv) {
  // Extract chromRPM and chromTRK from indiv
  std::vector<std::vector<int>> chromRPM = indiv.chromRPM;
  std::vector<std::vector<int>> chromTRK = indiv.chromTRK;

  // Reverse one subtour in chromRPM and chromTRK
  reverseOneSubtour(chromRPM);
  reverseOneSubtour(chromTRK);
  // Construct a new Individual with the modified chromRPM and chromTRK
  return {params, instance, chromRPM, chromTRK};
}

// single node insertion: choose one station from (1 to params.nbClient
// (including)) that is not in the route, insert it to a position other than the
// first and the end if every node is already in the route
void LocalSearch::insertOneNode(std::vector<std::vector<int>> &routes,
                                bool isRPM) {
  for (auto &route : routes) {
    // check the number of different node in the route
    std::unordered_set<int> nodeSet;
    for (auto node : route) {
      if (node != 0) {
        nodeSet.insert(node);
      }
    }
    if (nodeSet.size() == params.nbClients && isRPM) {
      continue;
    }
    if (route.size() > 3) {
      int index1 =
          getRand(route.size() - 2) + 1; // Adjusted to exclude depot nodes
      int node = getRand(params.nbClients) + 1;
      if (isRPM) {
        while (std::find(route.begin(), route.end(), node) != route.end()) {
          node = getRand(params.nbClients) + 1;
        }
        if (instance.networkInfo[node].brokenBike != 0)
          route.insert(route.begin() + index1, node);
      } else {
        node = getRand(params.nbClients + 1);
        if (instance.networkInfo[node].usableBike !=
            instance.networkInfo[node].targetUsable)
          route.insert(route.begin() + index1, node);
      }
    }
  }
}

Individual LocalSearch::move_6(Individual &indiv) {
  // Extract chromRPM and chromTRK from indiv
  std::vector<std::vector<int>> chromRPM = indiv.chromRPM;
  std::vector<std::vector<int>> chromTRK = indiv.chromTRK;

  // Insert one node in chromRPM and chromTRK
  insertOneNode(chromRPM, true);
  insertOneNode(chromTRK, false);

  // Construct a new Individual with the modified chromRPM and chromTRK
  return {params, instance, chromRPM, chromTRK};
}

// multiple node insertion: basically multiple time single node insertion, the
// time is decided using a random number generator
void LocalSearch::insertMultipleNodes(std::vector<std::vector<int>> &routes,
                                      bool isRPM) {
  for (auto &route : routes) {
    if (route.size() >= 3) {
      // generate the number of nodes to insert
      int numNodes = getRand(route.size() - 2) + 1;
      // call insertOneNode for numNodes times
      for (int i = 0; i < numNodes; i++) {
        insertOneNode(routes, isRPM);
      }
    }
  }
}

Individual LocalSearch::move_7(Individual &indiv) {
  // Extract chromRPM and chromTRK from indiv
  std::vector<std::vector<int>> chromRPM = indiv.chromRPM;
  std::vector<std::vector<int>> chromTRK = indiv.chromTRK;
  // Insert multiple nodes in chromRPM and chromTRK
  insertMultipleNodes(chromRPM, true);
  insertMultipleNodes(chromTRK, false);

  // Construct a new Individual with the modified chromRPM and chromTRK
  return {params, instance, chromRPM, chromTRK};
}

// single node deletion: choose one station from the route other than the first
// and the end, delete it
void LocalSearch::deleteOneNode(std::vector<std::vector<int>> &routes) {
  for (auto &route : routes) {
    if (route.size() > 3) {
      int index1 = getRand(route.size() - 2) + 1;
      route.erase(route.begin() + index1);
    }
  }
}

Individual LocalSearch::move_8(Individual &indiv) {
  // Extract chromRPM and chromTRK from indiv
  std::vector<std::vector<int>> chromRPM = indiv.chromRPM;
  std::vector<std::vector<int>> chromTRK = indiv.chromTRK;

  // Delete one node in chromRPM and chromTRK
  deleteOneNode(chromRPM);
  deleteOneNode(chromTRK);

  // Construct a new Individual with the modified chromRPM and chromTRK
  return {params, instance, chromRPM, chromTRK};
}

// multiple node deletion: basically multiple time single node deletion, the
// time is decided using a random number generator
void LocalSearch::deleteMultipleNodes(std::vector<std::vector<int>> &routes) {
  for (auto &route : routes) {
    // generate the number of nodes to delete
    if (route.size() >= 3) {
      int numNodes = getRand(route.size() - 2) + 1;
      // call deleteOneNode for numNodes times
      for (int i = 0; i < numNodes; i++) {
        deleteOneNode(routes);
      }
    }
  }
}

Individual LocalSearch::move_9(Individual &indiv) {
  // Extract chromRPM and chromTRK from indiv
  std::vector<std::vector<int>> chromRPM = indiv.chromRPM;
  std::vector<std::vector<int>> chromTRK =
      indiv.chromTRK; // Delete multiple nodes in chromRPM and chromTRK
  deleteMultipleNodes(chromRPM);
  deleteMultipleNodes(chromTRK);

  // Construct a new Individual with the modified chromRPM and chromTRK
  return {params, instance, chromRPM, chromTRK};
}

void LocalSearch::display2DVector(const std::vector<std::vector<int>> &routes) {
  for (const auto &route : routes) {
    for (const auto &node : route) {
      std::cout << node << " ";
    }
    std::cout << std::endl;
  }
}

Individual LocalSearch::executeAndDisplayMove(
    const std::function<Individual(Individual &)> &moveFunc, Individual &indiv,
    const std::string &moveId) {
  // Extract chromRPM and chromTRK from indiv
  std::vector<std::vector<int>> chromRPM = indiv.chromRPM;
  std::vector<std::vector<int>> chromTRK = indiv.chromTRK;

  // Perform the move operation
  Individual newIndiv = moveFunc(indiv);

  return newIndiv;
}
