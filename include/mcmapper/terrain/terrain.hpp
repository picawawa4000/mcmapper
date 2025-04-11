#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include <mcmapper/rng/noises.hpp>
#include <mcmapper/terrain/internoise.hpp>

struct TerrainGeneratorConfig {
    // Try to use the other generator if you already have an instance of `Noises`.
    TerrainGeneratorConfig(u64 seed, Dimension dimension);
    TerrainGeneratorConfig(std::shared_ptr<Noises> noises, u64 seed, Dimension dimension);

    std::shared_ptr<Noises> noises;
    std::unique_ptr<InterpolatedNoise> interNoise;
};

f64 sampleInitialDensity(Noises& noises, f64 x, f64 y, f64 z);
f64 sampleFinalDensity(TerrainGeneratorConfig& config, f64 x, f64 y, f64 z);

#endif