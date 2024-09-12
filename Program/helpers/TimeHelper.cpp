#include "TimeHelper.h"

namespace TimeHelper {
    int maxRepairQByTime(Params &params, double &remainingTime) {
        return static_cast<int>(std::floor(remainingTime / params.repairTime));
    }

    int maxLoadQByTime(Params &params, double &remainingTime) {
        return static_cast<int>(std::floor(remainingTime / params.loadingTime / 2));
    }
} // namespace TimeHelper