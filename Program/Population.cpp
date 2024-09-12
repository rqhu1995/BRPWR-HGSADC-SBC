#include "Population.h"
#include "LocalSearch.h"
#include <random>
#include <unordered_set>

Population::Population(Instance &instance) : instance(instance) {
}

void Population::generateInitialPopulation(Params &params) {
    int subPopMaxSize = params.ap.mu + params.ap.lambda;
    for (int i = 0; i < 4 * params.ap.mu
                    && (i == 0 || params.timeBudget == 0
                        || std::chrono::duration_cast<std::chrono::seconds>(
                               std::chrono::high_resolution_clock ::now() - params.startTime)
                                   .count()
                               < params.timeBudget);
        i++) {
        Individual randomIndiv(params, instance);
        // check if the start and end of the truck routes are all 0
        educateAndRepair(randomIndiv, params);
        if (feasibleSolution.size() >= subPopMaxSize || infeasibleSolution.size() >= subPopMaxSize) {
            sizeControl(subPopMaxSize, params);
        }
    }
}

void Population::educateAndRepair(Individual &indiv, Params &params) {
    auto localSearch = LocalSearch(params, instance);
    localSearch.run(indiv);
    // generate a random number between 0 and 1, if it is less than 0.5, then run
    // the local search again
    addIndividualToSubpopulation(indiv, params);
    if (!indiv.eval.isFeasible) {
        std::uniform_real_distribution<> dis(0, 1);
        double randomNum = dis(params.ran);
        if (randomNum < 0.5) {
            params.ap.penaltyCapacity *= 10;
            // std::cout << "repairing..." << std::endl;
            localSearch.run(indiv);
            // std::cout << "repair done!" << std::endl;
            addIndividualToSubpopulation(indiv, params);
            if (!indiv.eval.isFeasible) {
                params.ap.penaltyCapacity *= 10;
                // std::cout << "failed repairing repair again" << std::endl;
                localSearch.run(indiv);
                // std::cout << "failed repairs repair done!" << std::endl;
                addIndividualToSubpopulation(indiv, params);
            }
            params.ap.penaltyCapacity = penaltyCoefficient;
        }
    }
}

void Population::sizeControl(int subPopMaxSize, Params &params) {
    if (feasibleSolution.size() >= subPopMaxSize) {
        getBiasedFitness(feasibleSolution);
        std::sort(feasibleSolution.begin(), feasibleSolution.end(),
            [](Individual &a, Individual &b) { return a.eval.biasedFitness < b.eval.biasedFitness; });
        feasibleSolution.resize(params.ap.mu);
        // std::cout << this->feasibleSolution.size() << std::endl;
    }
    if (infeasibleSolution.size() >= subPopMaxSize) {
        getBiasedFitness(infeasibleSolution);
        std::sort(infeasibleSolution.begin(), infeasibleSolution.end(),
            [](Individual &a, Individual &b) { return a.eval.biasedFitness < b.eval.biasedFitness; });
        infeasibleSolution.resize(params.ap.mu);
        // std::cout << this->infeasibleSolution.size() << std::endl;
    }
}

double Population::adjustmentBasedOnProportion(Params &params) {
    int feasibleSize = latestLoadOfFeasibleSolution.size();
    int infeasibleSize = latestLoadOfInfeasibleSolution.size();
    // print both sizes
    double multiplier = 1.0;
    if (feasibleSize + infeasibleSize > 0) {
        double proportion = (double)feasibleSize / (double)(feasibleSize + infeasibleSize);
        if (proportion < params.ap.targetFeasible - 0.05) {
            multiplier = 1.2;
        } else if (proportion > params.ap.targetFeasible + 0.05) {
            multiplier = 0.85;
        }
    }
    if (multiplier != 1.0) {
        params.ap.penaltyCapacity *= multiplier;
        for (Individual &indiv : feasibleSolution) { indiv.solutionEvaluation(params); }
        for (Individual &indiv : infeasibleSolution) { indiv.solutionEvaluation(params); }
        penaltyCoefficient = params.ap.penaltyCapacity;
    }
    return multiplier;
}

// The similarity of a solution in a subpopulation is the sum of the number of
// common arcs between the truck routes in the solution and the union of truck
// routes from every other solution in the corresponding subpopulation and the
// number of common arcs between repairman routes in the solution and the union
// of repairman routes from every other solution in the corresponding
// subpopulation.
void getSimilarity(std::vector<Individual> &subpopulation) {
    // get all the arcs of the trucks and repairmen in all solutions
    // Define a hash function for std::pair<int, int>
    struct pair_hash {
        int operator()(const std::pair<int, int> &p) const {
            int h1 = std::hash<int>{}(p.first);
            int h2 = std::hash<int>{}(p.second);

            // Mainly for demonstration purposes, i.e. works but is overly simple
            // In the real world, use sth. like boost.hash_combine
            return h1 ^ h2;
        }
    };

    // Use unordered_set with custom hash function instead of set
    std::unordered_set<std::pair<int, int>, pair_hash> truckArcs;
    std::unordered_set<std::pair<int, int>, pair_hash> repairmanArcs;
    for (Individual &indiv : subpopulation) {
        for (auto &i : indiv.chromTRK) {
            for (int j = 0; j < i.size() - 1; j++) { truckArcs.insert(std::make_pair(i[j], i[j + 1])); }
        }
        for (auto &i : indiv.chromRPM) {
            for (int j = 0; j < i.size() - 1; j++) { repairmanArcs.insert(std::make_pair(i[j], i[j + 1])); }
        }
    }

    // calculate the similarity one individual by one individual
    for (Individual &indiv : subpopulation) {
        int truckSimilarity = 0;
        int repairmanSimilarity = 0;
        for (auto &i : indiv.chromTRK) {
            for (int j = 0; j < i.size() - 1; j++) {
                if (truckArcs.find(std::make_pair(i[j], i[j + 1])) != truckArcs.end()) { truckSimilarity++; }
            }
        }
        for (auto &i : indiv.chromRPM) {
            for (int j = 0; j < i.size() - 1; j++) {
                if (repairmanArcs.find(std::make_pair(i[j], i[j + 1])) != repairmanArcs.end()) {
                    repairmanSimilarity++;
                }
            }
        }
        indiv.eval.similarity = truckSimilarity + repairmanSimilarity;
    }
}

// biased fitness value is defined as the rank of the objval of the solution in
// the subpopulation r, plus (1-nelite/nindiv)*rank of the similarity of the
// solution in the population (both ranks are in the ascending order)
void getBiasedFitness(std::vector<Individual> &subpopulation) {
    int nbElite = 0;
    int nbIndiv = subpopulation.size();

    // sort the subpopulation by objval
    std::sort(subpopulation.begin(), subpopulation.end(),
        [](Individual &a, Individual &b) { return a.eval.objVal < b.eval.objVal; });

    std::vector<int> ranksFit(subpopulation.size());
    for (int i = 0; i < subpopulation.size(); i++) { ranksFit[i] = i + 1; }

    // sort the subpopulation by similarity
    std::sort(subpopulation.begin(), subpopulation.end(),
        [](Individual &a, Individual &b) { return a.eval.similarity < b.eval.similarity; });

    for (int i = 0; i < subpopulation.size(); i++) {
        subpopulation[i].eval.rankSim = i + 1;
        subpopulation[i].eval.rankFit = ranksFit[i];
        subpopulation[i].eval.biasedFitness
            = subpopulation[i].eval.rankFit + (1 - (double)nbElite / (double)nbIndiv) * subpopulation[i].eval.rankSim;
    }
}

/**
 * In the diversification, the best mu/3 solutions of each subpopulation are
 * preserved, and 4mu new individuals are generated, followed by the survivor
 * selection to adjust the size of each subpopulation to mu.
 */
void Population::diversification(Params &params) {
    // std::cout << "diversifying..." << std::endl;
    // preserve the best mu/3 solutions of each subpopulation
    std::sort(feasibleSolution.begin(), feasibleSolution.end(),
        [](Individual &a, Individual &b) { return a.eval.biasedFitness < b.eval.biasedFitness; });
    std::sort(infeasibleSolution.begin(), infeasibleSolution.end(),
        [](Individual &a, Individual &b) { return a.eval.biasedFitness < b.eval.biasedFitness; });
    if (feasibleSolution.size() > params.ap.mu / 3) feasibleSolution.resize(params.ap.mu / 3);
    if (infeasibleSolution.size() > params.ap.mu / 3) infeasibleSolution.resize(params.ap.mu / 3);

    // generate 4mu new individuals
    std::vector<Individual> newFeasibleSolutionCandidate;
    std::vector<Individual> newInfeasibleSolutionCandidate;
    int totalSize = feasibleSolution.size() + infeasibleSolution.size();
    while ((newFeasibleSolutionCandidate.size() + feasibleSolution.size() <= params.ap.mu)
           || (newInfeasibleSolutionCandidate.size() + infeasibleSolution.size() <= params.ap.mu))
    // while (totalSize <= 4 * params.ap.mu)
    {
        if (totalSize >= 4 * params.ap.mu) { break; }
        Individual randomIndiv(params, instance);
//         educateAndRepair(randomIndiv, params);

        // Return an empty vector if the type is not 1 or 0
        // add the solution to the corresponding subpopulation if it is not in the
        // subpopulation
        if (randomIndiv.eval.isFeasible && std::find (feasibleSolution.begin(), feasibleSolution.end(), randomIndiv) == feasibleSolution.end()) {
            newFeasibleSolutionCandidate.push_back(randomIndiv);
            ++totalSize;
        }
        if (!randomIndiv.eval.isFeasible && std::find (infeasibleSolution.begin(), infeasibleSolution.end(), randomIndiv) == infeasibleSolution.end()) {
            newInfeasibleSolutionCandidate.push_back(randomIndiv);
            ++totalSize;
        }
    }
    getSimilarity(newFeasibleSolutionCandidate);
    getBiasedFitness(newFeasibleSolutionCandidate);
    getSimilarity(newInfeasibleSolutionCandidate);
    getBiasedFitness(newInfeasibleSolutionCandidate);
    // sort newFeasibleSolutionCandidate and newInfeasibleSolutionCandidate by
    // objval
    std::sort(newFeasibleSolutionCandidate.begin(), newFeasibleSolutionCandidate.end(),
        [](Individual &a, Individual &b) { return a.eval.biasedFitness < b.eval.biasedFitness; });
    std::sort(newInfeasibleSolutionCandidate.begin(), newInfeasibleSolutionCandidate.end(),
        [](Individual &a, Individual &b) { return a.eval.biasedFitness < b.eval.biasedFitness; });

    while (feasibleSolution.size() <= params.ap.mu && newFeasibleSolutionCandidate.size() != 0) {
        // std::cout << "feasibleSolution.size() = " << feasibleSolution.size() <<
        // std::endl;
        feasibleSolution.push_back(newFeasibleSolutionCandidate.back());
        newFeasibleSolutionCandidate.pop_back();
    }

    while (infeasibleSolution.size() <= params.ap.mu && newInfeasibleSolutionCandidate.size() != 0) {
        infeasibleSolution.push_back(newInfeasibleSolutionCandidate.back());
        newInfeasibleSolutionCandidate.pop_back();
    }

    // std::cout << "diverse done" << std::endl;
}

Individual Population::getBinaryTournament(Params &params) {
    // Create a distribution that covers the entire population
    std::uniform_int_distribution<> distr(0, int(feasibleSolution.size() + infeasibleSolution.size()) - 1);

    // Pick two distinct indices
    int index1 = distr(params.ran);
    int index2 = distr(params.ran);
    while (index1 == index2) { index2 = distr(params.ran); }

    // Access the corresponding individuals
    Individual &indiv1 = (index1 < feasibleSolution.size()) ? feasibleSolution[index1]
                                                            : infeasibleSolution[index1 - feasibleSolution.size()];
    Individual &indiv2 = (index2 < feasibleSolution.size()) ? feasibleSolution[index2]
                                                            : infeasibleSolution[index2 - feasibleSolution.size()];

    // Return the one with the better fitness
    return (indiv1.eval.biasedFitness < indiv2.eval.biasedFitness) ? indiv1 : indiv2;
}

void Population::addIndividualToSubpopulation(Individual &indiv, Params &params) {
    bool somethingChanged = false;

    // Helper lambda function to avoid code duplication
    auto addIndividual
        = [&](std::vector<Individual> &solution, std::vector<Individual> &latestLoad, Individual &indiv) {
              if (std::find(solution.begin(), solution.end(), indiv) == solution.end()) {
                  solution.push_back(indiv);
                  latestLoad.push_back(indiv);
                  return true;
              }
              return false;
          };

    if (indiv.eval.isFeasible) {
        somethingChanged = addIndividual(feasibleSolution, latestLoadOfFeasibleSolution, indiv);
    } else
        somethingChanged = addIndividual(infeasibleSolution, latestLoadOfInfeasibleSolution, indiv);

    if (somethingChanged) updateAllBiasedFitness();
}

void Population::updateAllBiasedFitness() {
    getSimilarity(feasibleSolution);
    getBiasedFitness(feasibleSolution);
    getSimilarity(infeasibleSolution);
    getBiasedFitness(infeasibleSolution);
}