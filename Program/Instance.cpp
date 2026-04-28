//
// Created by Runqiu on 09/10/2023.
// Unified-HDF5 loader (Tier B-0, schema v1). Replaces per-station txt reads.
//

#include "Instance.h"
#include <cmath>
#include <climits>
#include <highfive/H5File.hpp>


namespace {

// Replace NaN entries with 0.0 to preserve downstream consumer semantics:
// dissatTable / priorityTable / priorityTableR originally used 0.0 as sentinel
// for (p + b > capacity) cells. The HDF5 schema uses NaN for that sentinel;
// convert back to 0.0 on load.
void ReplaceNanWithZero(std::vector<std::vector<double>> &table) {
    for (auto &row : table) {
        for (auto &v : row) {
            if (std::isnan(v)) { v = 0.0; }
        }
    }
}

// Slice a (N, C_max+1, C_max+1) stack into per-station (C_i+1, C_i+1) table,
// replacing NaN sentinels with 0.0.
std::vector<std::vector<double>> SliceAndDepad(
    const std::vector<std::vector<std::vector<double>>> &stack,
    int station_zero_based,
    int capacity_i) {
    const auto &slab = stack[station_zero_based];
    std::vector<std::vector<double>> out(
        capacity_i + 1, std::vector<double>(capacity_i + 1, 0.0));
    for (int p = 0; p <= capacity_i; ++p) {
        for (int b = 0; b <= capacity_i; ++b) {
            double v = slab[p][b];
            out[p][b] = std::isnan(v) ? 0.0 : v;
        }
    }
    return out;
}

}  // namespace


Instance::Instance(int nbClient, const std::string &instNo, double proportion,
                   bool expIsProportion) {
    const std::string h5_path =
        "../Instances_h5/" + std::to_string(nbClient) + "_" + instNo + ".h5";

    isProportion = expIsProportion;
    nbClients = nbClient;
    proportionRatio = proportion;

    HighFive::File file(h5_path, HighFive::File::ReadOnly);

    // --- time_matrix: (N+1) x (N+1), seconds ---
    file.getDataSet("/time_matrix").read(dist_mtx);

    // --- /stations per-field 1D arrays ---
    std::vector<int> cap_arr;
    std::vector<int> cur_usable;
    std::vector<int> cur_broken;
    std::vector<int> target_usable;
    file.getDataSet("/stations/capacity").read(cap_arr);
    file.getDataSet("/stations/current_usable").read(cur_usable);
    file.getDataSet("/stations/current_broken").read(cur_broken);
    file.getDataSet("/stations/target_usable").read(target_usable);

    networkInfo.reserve(nbClients + 1);
    // Depot at index 0 (sentinel: usable=INT_MAX, broken=0, target=INT_MAX, cap=INT_MAX)
    networkInfo.emplace_back(0, INT_MAX, 0, INT_MAX, INT_MAX);

    for (int i = 0; i < nbClients; ++i) {
        const int id = i + 1;
        const int capacity = cap_arr[i];
        const int usable = cur_usable[i];
        const int target = target_usable[i];
        int broken = cur_broken[i];

        if (isProportion) {
            if (usable > target) {
                broken = 0;
            } else {
                broken = static_cast<int>(std::ceil(
                    proportionRatio * (target - usable)));
                broken = std::min(broken, capacity - usable);
            }
        }
        networkInfo.emplace_back(id, usable, broken, target, capacity);
    }

    // --- /eudf/dissat_table, /eudf/bcrf_truck, /eudf/bcrf_repair ---
    // Each is (N, C_max+1, C_max+1) float64. Slice per-station, NaN → 0.0.
    std::vector<std::vector<std::vector<double>>> dissat_stack;
    std::vector<std::vector<std::vector<double>>> bcrf_truck_stack;
    std::vector<std::vector<std::vector<double>>> bcrf_repair_stack;
    file.getDataSet("/eudf/dissat_table").read(dissat_stack);
    file.getDataSet("/eudf/bcrf_truck").read(bcrf_truck_stack);
    file.getDataSet("/eudf/bcrf_repair").read(bcrf_repair_stack);

    // Legacy indexing:
    //   dissatTable: index 0 is a placeholder, stations are dissatTable[1..N]
    //   priorityTable / priorityTableR: 0-indexed, stations are [0..N-1]
    dissatTable.emplace_back();  // 1-based placeholder

    for (int i = 0; i < nbClients; ++i) {
        const int cap_i = cap_arr[i];
        dissatTable.push_back(SliceAndDepad(dissat_stack, i, cap_i));
        priorityTable.push_back(SliceAndDepad(bcrf_truck_stack, i, cap_i));
        priorityTableR.push_back(SliceAndDepad(bcrf_repair_stack, i, cap_i));
    }
}

// Legacy stubs — left in place for ABI compatibility with the header, but
// no longer invoked. The constructor now reads everything from one HDF5 file.
void Instance::readMatrixFromFile(const std::string & /*filepath*/) {}
void Instance::readStationInfoFromFile(const std::string & /*filepath*/,
                                       bool /*proportion*/) {}
void Instance::readDissatisTable(const std::string & /*filepath*/) {}
void Instance::readPriorityTable(const std::string & /*filepath*/) {}
void Instance::readPriorityTableR(const std::string & /*filepath*/) {}
