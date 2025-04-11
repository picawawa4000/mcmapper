#ifndef INTERNOISE_HPP
#define INTERNOISE_HPP

#include <array>

#include <mcmapper/rng/noise.hpp>

struct InterpolatedNoise {
    InterpolatedNoise(double xzScale, double yScale, double xzFactor, double yFactor, double smearScaleMultiplier);

    double sample(double x, double y, double z);

private:
    double xzScale, yScale, xzFactor, yFactor, smearScaleMultiplier;
    double scaledXzScale, scaledYScale;

    std::array<std::shared_ptr<PerlinNoise>, 16> upperInterpolatedNoise, lowerInterpolatedNoise;
    std::array<std::shared_ptr<PerlinNoise>, 8> interpolationNoise;
};

#endif