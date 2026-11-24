#ifndef SIM1_FLIGHT_FORCES_H
#define SIM1_FLIGHT_FORCES_H

#include "flight_dynamics.h"

namespace sim1 {

// Reconstructed prefix of the interface used by the force-collection loop.
class ForceContributor {
public:
    virtual void Evaluate(double step, Vector3* force, Vector3* moment) = 0;
};

// Reconstructed minimum layout for the callback adapter with table 0x20bbefd4.
// The bound member call uses the x86 thiscall ABI.
class BoundForceContributor : public ForceContributor {
public:
#ifdef SIM1_FORCE_COMPARISON
    __declspec(dllexport)
#endif
    virtual void Evaluate(double step, Vector3* force, Vector3* moment);

private:
    void* context;
    void (__thiscall* callback)(void*, double, Vector3*, Vector3*);
};

struct ForceContributorEntry {
    char registrationKey[0x80];
    ForceContributor* contributor;
};

struct ForceContributors {
    void Accumulate(double step, Vector3* force, Vector3* moment);

    int count;
    char unknown04[8];
    ForceContributorEntry* entries;
};

typedef char CheckForceContributorStride[sizeof(ForceContributorEntry) == 0x84 ? 1 : -1];
typedef char CheckForceContributorOffset[offsetof(ForceContributorEntry, contributor) == 0x80 ? 1 : -1];
typedef char CheckForceEntriesOffset[offsetof(ForceContributors, entries) == 0x0c ? 1 : -1];

}

#endif
