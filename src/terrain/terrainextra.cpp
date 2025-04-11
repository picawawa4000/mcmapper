#include <mcmapper/terrain/terrainextra.hpp>

#include <mcmapper/terrain/internoise.hpp>

static double sampleBase3dNoiseNether(double x, double y, double z, u64 seed) {
    static InterpolatedNoise noise(0.25, 0.375, 80.0, 60.0, 8.0);
    
}

double sampleDensityNether(double x, double y, double z, u64 seed) {
    return 0.0;
}

double sampleDensityEnd(double x, double y, double z, u64 seed) {
    return 0.0;
}
