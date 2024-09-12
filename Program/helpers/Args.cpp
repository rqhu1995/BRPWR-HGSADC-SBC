#include "Args.h"
#include "cmdparser.hpp"

// options: --nbStns (default 10, options 10, 20, 30, 60, 120, 200, 300, 400,
// 500), --nbTrk (default 1, options 1, 2, 3, 4, 5), --nbRpm (default 1, options
// 1, 2, 3), --nbInst (default 1, options 0 to 100), --penalty (default 10,
// options 10, 50, 100, 150, 200, 300, 400, 500, 1000). --nbNoImp (default 5000,
// options any positive integer larger than 5000)

namespace Args {
    // Declare functions and variables to be used elsewhere
    int nbStns;
    int nbTrk;
    int nbRpm;
    int nbInst;
    double brokenProp;
    int nbIterPenaltyManagement;
    double targetFeasible;
    int nbIterNoImp;
    double timeLimit;
    int penaltyCapacity;
    int lambda;
    int mu;
    int loadingTime;
    int repairTime;
    int vehicleCapacity;
    bool isProportion;
    double timeBudget;
    int itEDU;

    void parseArgs(int argc, char *argv[]) {
        cli::Parser parser(argc, argv);
        // ==== params related to the problem ====
        parser.set_optional<int>("ns", "num_stations", 10, "number of stations");
        parser.set_optional<int>("ntrk", "num_trucks", 1, "number of trucks");
        parser.set_optional<int>("nrpm", "num_repairer", 1, "number of repairers");
        parser.set_optional<int>("i", "inst_no", 1, "the instance id");
        parser.set_optional<int>("ldT", "loading_time", 60, "loading time");
        parser.set_optional<int>("rpT", "repair_time", 300, "repair time");
        parser.set_optional<int>("vcap", "vehicle_capacity", 25, "vehicle capacity");
        parser.set_optional<double>("tb", "time_budget", -1, "time budget for the repositioning");

        // ==== params related to the algorithm ====
        parser.set_optional<int>("pnt", "penalty", 10, "initial penalty for each violation of the constraints");
        parser.set_optional<int>("noimp", "num_non_improve", 5000, "number of iterations without improvement");
        parser.set_optional<double>("bprop", "broken_proportion", -0.5, "broken bike proportion");
        parser.set_optional<int>(
            "num_pnt_manage", "num_penalty_management", 100, "number of iterations for penalty management");
        parser.set_optional<double>(
            "fesP", "target_feasible", 0.2, "target feasible solution percentage in the population");
        parser.set_optional<double>(
            "tl", "timeLimit", 7200, "time limit for the algorithm as one of the terminating criteria");
        parser.set_optional<int>("mu", "mu", 25, "mu");
        parser.set_optional<int>("lambda", "lambda", 40, "lambda");
        parser.set_optional<int>("edu", "itedu", 40, "number of iterations repeated when an operator improved a solution");
        parser.set_optional<bool>("help", "help", false, "show help message");
        parser.run_and_exit_if_error();

        // Assign the values to the variables
        nbStns = parser.get<int>("ns");
        nbTrk = parser.get<int>("ntrk");
        nbRpm = parser.get<int>("nrpm");
        nbInst = parser.get<int>("i");
        penaltyCapacity = parser.get<int>("pnt");
        nbIterNoImp = parser.get<int>("noimp");
        brokenProp = parser.get<double>("bprop");
        nbIterPenaltyManagement = parser.get<int>("num_pnt_manage");
        targetFeasible = parser.get<double>("fesP");
        timeLimit = parser.get<double>("tl");
        mu = parser.get<int>("mu");
        lambda = parser.get<int>("lambda");
        loadingTime = parser.get<int>("ldT");
        repairTime = parser.get<int>("rpT");
        vehicleCapacity = parser.get<int>("vcap");
        itEDU = parser.get<int>("edu");
        isProportion = brokenProp >= 0;
        // if timeBudget is not provided, then timeBudget is set based on the number of stations, the mapping is as follows:
        // 6, 10, or 15: 7200; 20,30: 10800; 60,120: 14400; 200,300,400,500: 18000
        timeBudget = parser.get<double>("tb");
        if (timeBudget < 0) {
            if (nbStns <= 15) {
                timeBudget = 7200;
            } else if (nbStns <= 30) {
                timeBudget = 10800;
            } else if (nbStns <= 120) {
                timeBudget = 14400;
            } else {
                timeBudget = 18000;
            }
        } else {
            std::cout << "Time budget is set to " << timeBudget << std::endl;
        }
    }
} // namespace Args