#pragma once

#include "../Params.h"
namespace TimeHelper {
    int maxRepairQByTime(Params &params, double &remainingTime);
    int maxLoadQByTime(Params &params, double &remainingTime);
} // namespace TimeHelper
