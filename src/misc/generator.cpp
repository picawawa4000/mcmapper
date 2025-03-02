#include <mcmapper/misc/generator.hpp>
#include <mcmapper/terrain/spline.hpp>

typedef std::array<f32, 16> Samples2d;
typedef std::array<std::array<f32, 96>, 16> Samples3d;
typedef std::array<std::array<Biome, 96>, 16> Biomes3d;

Samples2d populateNoise2d(const DoublePerlinNoise& noise, i32 chunkX, i32 chunkZ) {
    i32 originX = chunkX << 2, originZ = chunkZ << 2;

    Samples2d samples;

    for (int i = 0; i < 16; ++i)
        samples[i] = noise.sample(originX + std::floor(i / 4), 0, originZ + std::floor(i % 4));

    return samples;
}

Samples2d populatePv(const Samples2d& weirdness) {
    Samples2d ret;

    for (int i = 0; i < 16; ++i)
        ret[i] = pvTransform(weirdness[i]);
    
    return ret;
}

Samples2d populateOffset(const Samples2d& continentalness, const Samples2d& erosion, const Samples2d& weirdness, const Samples2d& pv) {
    Samples2d ret;

    for (int i = 0; i < 16; ++i)
        ret[i] = offsetSpline()->sample({continentalness[i], erosion[i], pv[i], weirdness[i]});

    return ret;
}

Samples3d populateDepth(const Samples2d& offset) {
    Samples3d ret;

    for (int i = 0; i < 16; ++i) {
        f32 columnOffset = offset[i];
        for (int j = 0; j < 96; ++j)
            ret[i][j] = columnOffset + yClampedGradient(j << 2, -64, 320, 1.5, -1.5);
    }

    return ret;
}

// This is a completely over-the-top solution to the speed problem.
void ChunkGenerator::populate() {
    Samples2d 
        temperatureSamples = populateNoise2d(this->noises->temperature, this->x, this->z),
        humiditySamples = populateNoise2d(this->noises->humidity, this->x, this->z),
        continentalnessSamples = populateNoise2d(this->noises->continentalness, this->x, this->z),
        erosionSamples = populateNoise2d(this->noises->erosion, this->x, this->z),
        weirdnessSamples = populateNoise2d(this->noises->weirdness, this->x, this->z),
        pvSamples = populatePv(weirdnessSamples),
        offsetSamples = populateOffset(continentalnessSamples, erosionSamples, weirdnessSamples, pvSamples);
    
    Samples3d depthSamples = populateDepth(offsetSamples);

    Biomes3d biomes;
    const std::shared_ptr<SearchTree> searchTree = getSearchTree();
    bool canUseSharedBiome = true;
    Biome sharedBiome = THE_VOID;

    for (int i = 0; i < 16; ++i) {
        NoisePoint point(temperatureSamples[i] * 10000, humiditySamples[i] * 10000, continentalnessSamples[i] * 10000, erosionSamples[i] * 10000, 0, weirdnessSamples[i] * 10000);

        for (int j = 0; j < 16; ++j) {
            point.depth = depthSamples[i][j] * 10000;
            Biome biome = searchTree->get(point);
            biomes[i][j] = biome;

            if (canUseSharedBiome) {
                if (sharedBiome == THE_VOID) sharedBiome = biome;
                if (sharedBiome != biome) canUseSharedBiome = false;
            }
        }
    }

    if (canUseSharedBiome) this->biome = sharedBiome;
    else {
        this->biome = THE_VOID;
        this->biomes = std::make_unique<Biomes3d>(biomes);
    }
}