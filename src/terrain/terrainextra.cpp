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
    if (config.dimension != Dimension::DIM_NETHER)
        throw std::runtime_error("Config used for Nether generation initialised for non-Nether dimension!");

    double baseDensity = config.interNoise->sample(x, y, z);
    double density = netherSlides(baseDensity, y);
    return squeeze(density * 0.64);
}

static inline double endSlides(double density, double y) {
    double upper = yClampedGradient(y, 56, 312, 1., 0.);
    double lower = yClampedGradient(y, 4, 32, 0., 1.);
    return lerp(lower, -0.234375, lerp(upper, -23.4375, density));
}

double sampleDensityEnd(TerrainGeneratorConfig& config, double x, double y, double z) {
    if (config.dimension != Dimension::DIM_END)
        throw std::runtime_error("Config used for End generation initialised for non-End dimension!");

    double baseDensity = config.interNoise->sample(x, y, z);
    double endIslands = config.endIslandsNoise->sample(x, z);
    double density = endSlides(baseDensity + endIslands, y);
    return squeeze(density * 0.64);
}
