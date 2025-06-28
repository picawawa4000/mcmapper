#include <mcmapper/terrain/internoise.hpp>

static std::array<std::shared_ptr<PerlinNoise>, 16> createLegacy(Random& rng) {
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

static std::array<std::shared_ptr<PerlinNoise>, 8> createInterpolationLegacy(Random& rng) {
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

InterpolatedNoise::InterpolatedNoise(Random& random, double xzScale, double yScale, double xzFactor, double yFactor, double smearScaleMultiplier)
    : xzFactor(xzFactor), yFactor(yFactor), xzScale(xzScale), yScale(yScale), smearScaleMultiplier(smearScaleMultiplier),
    scaledXzScale(684.412 * xzScale), scaledYScale(684.412 * yScale) {
    
    this->lowerInterpolatedNoise = createLegacy(random);
    this->upperInterpolatedNoise = createLegacy(random);
    this->interpolationNoise = createInterpolationLegacy(random);
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

void InterpolatedNoise::replace(Random& random) {
    this->lowerInterpolatedNoise = createLegacy(random);
    this->upperInterpolatedNoise = createLegacy(random);
    this->interpolationNoise = createInterpolationLegacy(random);
}

EndIslandsNoise::EndIslandsNoise(u64 seed) {
    CheckedRandom rng(seed);
    rng.skip(17292);

    // Construct internal simplex sampler
    this->originX = rng.next_f64() * 256.;
    this->originY = rng.next_f64() * 256.;
    this->originZ = rng.next_f64() * 256.;

    for (int i = 0; i < 256; ++i) this->permutation[i] = i;

    // Knuth shuffle (same as PerlinNoise ctor)
    for (int i = 0; i < 256; ++i) {
        u8 idx = (u8)rng.next_i32(256 - i);
        u8 temp = this->permutation[i];
        this->permutation[i] = this->permutation[i + idx];
        this->permutation[i + idx] = temp;
    }
}

double EndIslandsNoise::sample(double x, double z) {
    i32 blockX = (i32)x / 8;
    i32 blockZ = (i32)z / 8;

    i32 sectionX = blockX / 2;
    i32 sectionZ = blockZ / 2;
    i32 localX = blockX | 1;
    i32 localZ = blockZ | 1;
    float f = 100. - std::sqrt(blockX * blockX + blockZ * blockZ) * 8.;
    f = std::clamp(f, -100.f, 80.f);
    
    for (int xOffset = -12; xOffset <= 12; ++xOffset) {
        for (int zOffset = -12; zOffset <= 12; ++zOffset) {
            i32 newX = sectionX + xOffset;
            i32 newZ = sectionZ + zOffset;
            if (newX * newX + newZ * newZ <= 4096 || !(this->sampleSimplex(newX, newZ) < -0.9f)) continue;
            float g = std::fmod(std::abs(newX) * 3439.0f + std::abs(newZ) * 147.f, 13.f) + 9.f;
            float h = localX - xOffset * 2;
            float i = localZ - zOffset * 2;
            float newF = 100.f - std::sqrt(h * h + i * i) * g;
            newF = std::clamp(newF, -100.f, 80.f);
            f = std::max(f, newF);
        }
    }
    
    return (f - 8.) / 128.;
}

const i32 gradients[16][2] = {
    {1, 1}, 
    {-1, 1}, 
    {1, -1}, 
    {-1, -1}, 
    {1, 0}, 
    {-1, 0}, 
    {1, 0},
    {-1, 0}, 
    {0, 1}, 
    {0, -1}, 
    {0, 1}, 
    {0, -1}, 
    {1, 1}, 
    {0, -1}, 
    {-1, 1}, 
    {0, -1}
};

static f64 dot(i32 hash, f64 x, f64 z) {
    const i32 * gradient = gradients[hash & 0xF];
    return gradient[0] * x + gradient[1] * z;
}

double EndIslandsNoise::grad(i32 hash, double x, double z, double distance) {
    double h = distance - x * x - z * z;
    if (h < 0.) return 0.;
    return h * h * h * h * dot(hash, x, z);
}

u8 EndIslandsNoise::map(i32 hash) {
    return this->permutation[hash & 0xff];
}

double EndIslandsNoise::sampleSimplex(i32 x, i32 z) {
    /// TODO: make all of these functions have descriptive names
    const double sqrt3 = std::sqrt(3.);
    const double skewFactor = 0.5 * (sqrt3 - 1.);
    const double unskewFactor = (3. - sqrt3) / 6.;

    double f = (x + z) * skewFactor;
    i32 i = std::floor(x + f);
    i32 j = std::floor(z + f);
    double g = (i + j) * unskewFactor;
    double h = i - g;

    double l = x - h;
    double k = j - g;
    double m = z - k;

    i32 n, o;
    if (l > m) {
        n = 1;
        o = 0;
    } else {
        n = 0;
        o = 1;
    }

    double p = l - n + unskewFactor;
    double q = m - o + unskewFactor;
    double r = l - 1. + 2. * unskewFactor;
    double s = m - 1. + 2. * unskewFactor;

    i32 t = i & 0xff;
    i32 u = j & 0xff;
    i32 v = this->map(t + this->map(u)) % 12;
    i32 w = this->map(t + n + this->map(u + o)) % 12;
    i32 y = this->map(t + 1 + this->map(u + 1)) % 12;

    double
        ret1 = this->grad(v, l, m, .5),
        ret2 = this->grad(w, p, q, .5),
        ret3 = this->grad(y, r, s, .5);
    
    return 70. * (ret1 + ret2 + ret3);
}
