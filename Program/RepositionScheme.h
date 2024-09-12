//
// Created by Transport02 on 27/11/2023.
//

#pragma once

struct RepositionSchemeBase {
    int station{};           // the station ID
    double arrivingTime = 0; // the time the entity arrives at the station

    virtual ~RepositionSchemeBase() = default; // virtual destructor for proper cleanup

    // Possibly other common methods, marked as virtual if they should be
    // overridden
};

struct RepositionSchemeTRK : public RepositionSchemeBase {
    int loadingQuantityU = 0;   // # of loaded usable bikes
    int loadingQuantityB = 0;   // # of loaded broken bikes
    int unloadingQuantityU = 0; // # of unloaded usable bikes
    int unloadingQuantityB = 0; // # of unloaded broken bikes
    int truckUQ = 0;            // truck usable bikes quantity when it leaves the station
    int truckBQ = 0;            // truck broken bikes quantity when it leaves the station

    // Additional truck-specific methods
};

struct RepositionSchemeRPM : public RepositionSchemeBase {
    int repairingQuantity = 0; // # of bikes repaired at this station

    // Additional repairman-specific methods
};

struct ReachableListCandidateTRK {
    int station = 0;
    int deliveryQuantityU = 0;
    int collectionQuantityU = 0;
    int deliveryQuantityB = 0;
    int collectionQuantityB = 0;
};
