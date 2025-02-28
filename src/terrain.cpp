#include <mcmapper/terrain.hpp>

#include <mcmapper/noise.hpp>

static double scaleCaves(double value) {
    if (value < -0.75) return 0.5;
    if (value < -0.5) return 0.75;
    if (value < 0.5) return 1.;
    if (value < 0.75) return 2.;
    return 3.;
}

static double scaleTunnels(double value) {
    if (value < -0.5) return 0.75;
    if (value < 0.) return 1.;
    if (value < 0.5) return 1.5;
    return 2.;
}

