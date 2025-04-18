#ifndef INTERNOISE_HPP
#define INTERNOISE_HPP

#include <array>

#include <mcmapper/rng/noise.hpp>

struct InterpolatedNoise {
    InterpolatedNoise(Random& random, double xzScale, double yScale, double xzFactor, double yFactor, double smearScaleMultiplier);

    double sample(double x, double y, double z);

private:
    double xzScale, yScale, xzFactor, yFactor, smearScaleMultiplier;
    double scaledXzScale, scaledYScale;

    std::array<std::shared_ptr<PerlinNoise>, 16> upperInterpolatedNoise, lowerInterpolatedNoise;
    std::array<std::shared_ptr<PerlinNoise>, 8> interpolationNoise;
};

// Encapsulates SimplexNoiseSampler.
struct EndIslandsNoise {
    explicit EndIslandsNoise(u64 seed);

    double sample(double x, double z);

private:
    // Internal SimplexNoiseSampler
    double grad(i32 hash, double x, double z, double distance);
    u8 map(i32 hash);
    double sampleSimplex(i32 x, i32 z);

    double originX, originY, originZ;
    u8 permutation[256];
};

#endif