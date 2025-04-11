#include <mcmapper/terrain/internoise.hpp>

static std::array<std::shared_ptr<PerlinNoise>, 16> createLegacy(XoroshiroRandom& rng) {
    const std::array<f64, 16> amplitudes = {1., 1., 1., 1., 1., 1., 1., 1., 1., 1., 1., 1., 1., 1., 1., 1.};
    const i32 firstOctave = -15;

    std::array<std::shared_ptr<PerlinNoise>, 16> octaves;

    i32 i = 0;
    i32 size = amplitudes.size();
    i32 lastOctave = firstOctave + size - 1;

    if (lastOctave == 0) {
        octaves[i] = std::shared_ptr<PerlinNoise>(new PerlinNoise(rng));
        ++i;
    } else rng.skip(-lastOctave * 262);

    for (; i < size; ++i) {
        if (amplitudes[i] != 0.) octaves[i] = std::shared_ptr<PerlinNoise>(new PerlinNoise(rng));
        else octaves[i] = nullptr;
    }

    return octaves;
}

static std::array<std::shared_ptr<PerlinNoise>, 8> createInterpolationLegacy(XoroshiroRandom& rng) {
    const std::array<f64, 8> amplitudes = {1., 1., 1., 1., 1., 1., 1., 1.};
    const i32 firstOctave = -7;

    std::array<std::shared_ptr<PerlinNoise>, 8> octaves;

    i32 i = 0;
    i32 size = amplitudes.size();
    i32 lastOctave = firstOctave + size - 1;

    if (lastOctave == 0) {
        octaves[i] = std::shared_ptr<PerlinNoise>(new PerlinNoise(rng));
        ++i;
    } else rng.skip(-lastOctave * 262);

    for (; i < size; ++i) {
        if (amplitudes[i] != 0.) octaves[i] = std::shared_ptr<PerlinNoise>(new PerlinNoise(rng));
        else octaves[i] = nullptr;
    }

    return octaves;
}

InterpolatedNoise::InterpolatedNoise(double xzFactor, double yFactor, double xzScale, double yScale, double smearScaleMultiplier)
    : xzFactor(xzFactor), yFactor(yFactor), xzScale(xzScale), yScale(yScale), smearScaleMultiplier(smearScaleMultiplier),
    scaledXzScale(684.412 * xzScale), scaledYScale(684.412 * yScale) {
    XoroshiroRandom rng(0);
    
    this->lowerInterpolatedNoise = createLegacy(rng);
    this->upperInterpolatedNoise = createLegacy(rng);
    this->interpolationNoise = createInterpolationLegacy(rng);
}

double InterpolatedNoise::sample(double x, double y, double z) {
    // I fully understand that this is ugly formatting.
    // But my desire to avoid ugly formatting is trumped
    // by my desire to not have to type out "double" 80 times.
    double
        scaledX = x * this->scaledXzScale,
        scaledY = y * this->scaledYScale,
        scaledZ = z * this->scaledXzScale;
    double
        factoredX = scaledX / this->xzFactor,
        factoredY = scaledY / this->yFactor,
        factoredZ = scaledZ / this->xzFactor;
    double smearScale = this->scaledYScale * this->smearScaleMultiplier;
    double factoredSmearScale = smearScale / this->yFactor;
    double interpolationTotal = 0.;
    double freq = 1.;
    for (int i = 0; i < 8; ++i) {
        std::shared_ptr<PerlinNoise> noise = this->interpolationNoise.at(i);
        if (noise) interpolationTotal += noise->sample(factoredX * freq, factoredY * freq, factoredZ * freq, factoredSmearScale * freq, factoredY * freq) / freq;
        freq /= 2.;
    }
    interpolationTotal = (interpolationTotal / 10. + 1.) / 2.;
    bool
        gt = interpolationTotal >= 1.,
        lt = interpolationTotal <= 0.;
    freq = 1.;
    double lowerTotal = 0., upperTotal = 0.;
    for (int i = 0; i < 16; ++i) {
        std::shared_ptr<PerlinNoise> upperNoise = this->upperInterpolatedNoise.at(i);
        std::shared_ptr<PerlinNoise> lowerNoise = this->lowerInterpolatedNoise.at(i);
        double x = scaledX * freq;
        double y = scaledY * freq;
        double z = scaledZ * freq;
        double yScale = smearScale * freq;
        if (!gt && lowerNoise)
            lowerTotal += lowerNoise->sample(x, y, z, yScale, y) / freq;
        if (!lt && upperNoise)
            upperTotal += upperNoise->sample(x, y, z, yScale, y) / freq;
        freq /= 2.;
    }
    return clampedLerp(lowerTotal / 512., upperTotal / 512., interpolationTotal) / 128.;
}
