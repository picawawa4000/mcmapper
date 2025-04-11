#include <mcmapper/terrain/terrainextra.hpp>

#include <mcmapper/terrain/internoise.hpp>

// 0, 128, 24, 0, 0.9375, -8, 24, 2.5
static inline double netherSlides(double density, double y) {
    double upper = yClampedGradient(y, 104, 128, 1., 0.);
    double lower = yClampedGradient(y, -8, 24, 0., 1.);
    return lerp(lower, 2.5, lerp(upper, 0.9375, density));
}

static inline double squeeze(double density) {
    double c = std::clamp(density, -1., 1.);
    return c / 2. - c * c * c / 24.;
}

double sampleDensityNether(TerrainGeneratorConfig& config, double x, double y, double z) {
    double baseDensity = config.interNoise->sample(x, y, z);
    double density = netherSlides(baseDensity, y);
    return squeeze(density * 0.64);
}

double sampleDensityEnd(TerrainGeneratorConfig& config, double x, double y, double z) {
    return 0.0;
}
