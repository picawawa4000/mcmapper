#include <mcmapper/biome/biomeextra.hpp>

#include <array>
#include <limits>

struct NetherNoisePoint {
    Biome b;
    // offset is squared
    double t, h, o;
};

const std::array<const NetherNoisePoint, 5> netherPoints = {{
    {.b=NETHER_WASTES, .t=0., .h=0., .o=0.},
    {.b=CRIMSON_FOREST, .t=.4, .h=0., .o=0.},
    {.b=SOUL_SAND_VALLEY, .t=0., .h=-.5, .o=0.},
    {.b=BASALT_DELTAS, .t=-.5, .h=0., .o=.175*.175},
    {.b=WARPED_FOREST, .t=0., .h=.5, .o=.375*.375},
}};

Biome getNether(double temperature, double humidity) {
    Biome bestBiome = THE_VOID;
    double bestDist = std::numeric_limits<double>::max();

    for (int i = 0; i < netherPoints.size(); ++i) {
        const NetherNoisePoint& point = netherPoints[i];
        double dt = temperature - point.t;
        double dh = humidity - point.h;
        double dist = dt * dt + dh * dh + point.o;
        if (dist < bestDist) {
            bestDist = dist;
            bestBiome = point.b;
        }
    }

    return bestBiome;
}

Biome getEnd(double erosion) {
    if (erosion < -0.21875) return SMALL_END_ISLANDS;
    if (erosion < -0.0625) return END_BARRENS;
    if (erosion < 0.25) return END_MIDLANDS;
    return END_HIGHLANDS;
}