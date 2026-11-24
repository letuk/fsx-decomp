#include "flight_forces.h"

namespace sim1 {

// api.dll ordinal 109 at sim1.dll IAT 0x20bb11f0.
extern "C" __declspec(dllimport) void __stdcall SimFillBytes(void* buffer, int value, unsigned bytes);
extern double gravityAcceleration;

// Original: sim1.dll 0x20bf6a20
void BoundForceContributor::Evaluate(double step, Vector3* force, Vector3* moment)
{
    callback(context, step, force, moment);
}

// Original: sim1.dll 0x20bccae0
__declspec(noinline) void ForceContributors::Accumulate(double step, Vector3* force, Vector3* moment)
{
    SimFillBytes(force, 0, sizeof(*force));
    SimFillBytes(moment, 0, sizeof(*moment));

    for (int index = 0; index < count; ++index) {
        Vector3 contributionForce = {0.0, 0.0, 0.0};
        Vector3 contributionMoment = {0.0, 0.0, 0.0};
        ForceContributor* contributor = entries[index].contributor;
        if (contributor != 0) {
            contributor->Evaluate(step, &contributionForce, &contributionMoment);
            force->x += contributionForce.x;
            force->y += contributionForce.y;
            force->z += contributionForce.z;
            moment->x += contributionMoment.x;
            moment->y += contributionMoment.y;
            moment->z += contributionMoment.z;
        }
    }
}

// Original: sim1.dll 0x20bccef0
__declspec(noinline) void MassProperties::Recalculate(int forceUpdate)
{
    if (forceUpdate == 0 && dirty == 0) {
        return;
    }

    const unsigned count = entryCount;
    totalWeight = 0.0f;
    mass = 0.0f;
    weightedPosition.x = 0.0f;
    weightedPosition.y = 0.0f;
    weightedPosition.z = 0.0f;
    inertiaX = baseInertiaX;
    inertiaY = baseInertiaY;
    inertiaZ = baseInertiaZ;
    crossInertia = baseInertiaYZ;
    inertiaZX = baseInertiaZX;
    inertiaXY = baseInertiaXY;

    if (count != 0) {
        WeightEntry* const* entry = entries;
        for (unsigned remaining = count; remaining != 0; --remaining, ++entry) {
            const WeightEntry* item = *entry;
            if (item == 0 || item->weight == 0 || item->position == 0) {
                continue;
            }

            totalWeight += *item->weight;
            weightedPosition.x += *item->weight * item->position->x;
            weightedPosition.y += item->position->y * *item->weight;
            weightedPosition.z += item->position->z * *item->weight;

            const float x = item->position->x - centerOfGravity.x;
            const float y = item->position->y - centerOfGravity.y;
            const float z = item->position->z - centerOfGravity.z;
            const float xSquared = x * x;
            const float ySquared = y * y;
            const float zSquared = z * z;
            const float itemMass = static_cast<float>(*item->weight / gravityAcceleration);
            inertiaX += itemMass * (ySquared + zSquared);
            inertiaY += itemMass * (xSquared + ySquared);
            inertiaZ += itemMass * (xSquared + zSquared);
            crossInertia += itemMass * y * z;
            inertiaZX += z * itemMass * x;
            inertiaXY += y * itemMass * x;
        }
    }

    centerOfGravity.x = weightedPosition.x / totalWeight;
    centerOfGravity.y = weightedPosition.y / totalWeight;
    centerOfGravity.z = weightedPosition.z / totalWeight;
    mass = static_cast<float>(totalWeight / gravityAcceleration);
    owner->positionDispatch->massPropertiesChanged(owner);
    dirty = 0;
}

}
