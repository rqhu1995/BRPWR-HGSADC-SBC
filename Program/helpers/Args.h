#pragma once
// options: --nbStns (default 10, options 10, 20, 30, 60, 120, 200, 300, 400,
// 500), --nbTrk (default 1, options 1, 2, 3, 4, 5), --nbRpm (default 1, options
// 1, 2, 3), --nbInst (default 1, options 0 to 100), --penalty (default 10,
// options 10, 50, 100, 150, 200, 300, 400, 500, 1000). --nbNoImp (default 5000,
// options any positive integer larger than 5000)

namespace Args {
    // Declare functions and variables to be used elsewhere
    extern int nbStns;
    extern int nbTrk;
    extern int nbRpm;
    extern int nbInst;
    extern double brokenProp;
    extern int vehicleCapacity;
    extern int repairTime;
    extern int loadingTime;
    extern double timeBudget;
    extern int mu;
    extern int lambda;
    extern int nbIterPenaltyManagement;
    extern double targetFeasible;
    extern int nbIterNoImp;
    extern double timeLimit;
    extern int penaltyCapacity;
    extern bool isProportion;
    extern int itEDU;

    void parseArgs(int argc, char *argv[]);
} // namespace Args