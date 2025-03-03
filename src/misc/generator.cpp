#include <mcmapper/misc/generator.hpp>
#include <mcmapper/terrain/spline.hpp>

#include <utility>

#include <chrono>

static Samples2d populateNoise2d(const DoublePerlinNoise& noise, i32 chunkX, i32 chunkZ) {
    std::chrono::high_resolution_clock clock;
    auto start = clock.now();

    i32 originX = chunkX << 2, originZ = chunkZ << 2;

    Samples2d samples;

    for (int i = 0; i < 16; ++i)
        samples[i] = noise.sample(originX + std::floor(i / 4), 0, originZ + std::floor(i % 4));

    auto end = clock.now();
    std::cout << "populateNoise2d: " << end - start << std::endl;

    return samples;
}

static Samples2d populatePv(const Samples2d& weirdness) {
    std::chrono::high_resolution_clock clock;
    auto start = clock.now();

    Samples2d ret;

    for (int i = 0; i < 16; ++i)
        ret[i] = pvTransform(weirdness[i]);
    
    auto end = clock.now();
    std::cout << "populatePv: " << end - start << std::endl;
    
    return ret;
}

static Samples2d populateOffset(const Samples2d& continentalness, const Samples2d& erosion, const Samples2d& weirdness, const Samples2d& pv) {  
    std::chrono::high_resolution_clock clock;
    auto start = clock.now();

    Samples2d ret;

    for (int i = 0; i < 16; ++i)
        ret[i] = offsetSpline()->sample({continentalness[i], erosion[i], pv[i], weirdness[i]});

    auto end = clock.now();
    std::cout << "populateOffset: " << end - start << std::endl;

    return ret;
}

static Samples3d populateDepth(const Samples2d& offset) {
    std::chrono::high_resolution_clock clock;
    auto start = clock.now();

    Samples3d ret;

    for (int i = 0; i < 16; ++i) {
        f32 columnOffset = offset[i];
        for (int j = 0; j < 96; ++j)
            ret[i][j] = columnOffset + yClampedGradient(j << 2, -64, 320, 1.5, -1.5);
    }

    auto end = clock.now();
    std::cout << "populateDepth: " << end - start << std::endl;

    return ret;
}

static std::pair<Biome, std::unique_ptr<Biomes3d>> populateBiomes(const Samples2d& temperature, const Samples2d& humidity, const Samples2d& continentalness, const Samples2d& erosion, const Samples2d& weirdness, const Samples3d& depth) {
    std::chrono::high_resolution_clock clock;
    auto start = clock.now();

    Biomes3d biomes;
    const std::shared_ptr<SearchTree> searchTree = getSearchTree();
    bool canUseSharedBiome = true;
    Biome sharedBiome = THE_VOID;

    for (int i = 0; i < 16; ++i) {
        NoisePoint point(temperature[i], humidity[i], continentalness[i], erosion[i], 0, weirdness[i]);

        for (int j = 0; j < 96; ++j) {
            point.depth = depth[i][j];
            Biome biome = searchTree->get(point);
            biomes[i][j] = biome;

            if (canUseSharedBiome) {
                if (sharedBiome == THE_VOID) sharedBiome = biome;
                else if (sharedBiome != biome) canUseSharedBiome = false;
            }
        }
    }

    auto end = clock.now();
    std::cout << "populateBiomes: " << end - start << std::endl;

    if (canUseSharedBiome) return {sharedBiome, std::unique_ptr<Biomes3d>()};
    return {THE_VOID, std::make_unique<Biomes3d>(biomes)};
}

// This is by far the slowest function.
static std::pair<Biome, std::unique_ptr<Biomes2d>> populateFlatBiomes(const Samples2d& temperature, const Samples2d& humidity, const Samples2d& continentalness, const Samples2d& erosion, const Samples2d& weirdness) {
    std::chrono::high_resolution_clock clock;
    auto start = clock.now();
    
    Biomes2d biomes;
    const std::shared_ptr<SearchTree> searchTree = getSearchTree();
    bool canUseSharedBiome = true;
    Biome sharedBiome = THE_VOID;

    for (int i = 0; i < 16; ++i) {
        NoisePoint point(temperature[i], humidity[i], continentalness[i], erosion[i], 0, weirdness[i]);

        Biome biome = searchTree->get(point);
        biomes[i] = biome;

        if (canUseSharedBiome) {
            if (sharedBiome == THE_VOID) sharedBiome = biome;
            else if (sharedBiome != biome) canUseSharedBiome = false;
        }
    }

    auto end = clock.now();
    std::cout << "populateFlatBiomes: " << end - start << std::endl;

    if (canUseSharedBiome) return {sharedBiome, std::unique_ptr<Biomes2d>()};
    return {THE_VOID, std::make_unique<Biomes2d>(biomes)};
}

// This is a completely over-the-top solution to the speed problem.
void ChunkGenerator::populate(const ChunkGeneratorFlags& flags) {
    Samples2d temperatureSamples, humiditySamples, continentalnessSamples, erosionSamples, weirdnessSamples, pvSamples, offsetSamples;
    Samples3d depthSamples;

    if (flags.biomes || flags.temperature) temperatureSamples = populateNoise2d(this->noises->temperature, this->x, this->z);
    if (flags.biomes || flags.humidity) humiditySamples = populateNoise2d(this->noises->humidity, this->x, this->z);
    if (flags.biomes || flags.continentalness || flags.offset || flags.depth) continentalnessSamples = populateNoise2d(this->noises->continentalness, this->x, this->z);
    if (flags.biomes || flags.erosion || flags.offset || flags.depth) erosionSamples = populateNoise2d(this->noises->erosion, this->x, this->z);
    if (flags.biomes || flags.weirdness || flags.pv || flags.offset || flags.depth) weirdnessSamples = populateNoise2d(this->noises->weirdness, this->x, this->z);
    if (flags.biomes || flags.pv || flags.offset || flags.depth) pvSamples = populatePv(weirdnessSamples);
    if ((flags.biomes && !flags.biomeSurfaceOnly) || flags.offset || flags.depth) offsetSamples = populateOffset(continentalnessSamples, erosionSamples, weirdnessSamples, pvSamples);
    if ((flags.biomes && !flags.biomeSurfaceOnly) || flags.depth) depthSamples = populateDepth(offsetSamples);

    if (flags.temperature) this->temperatureSamples = std::make_shared<Samples2d>(temperatureSamples);
    if (flags.humidity) this->humiditySamples = std::make_shared<Samples2d>(humiditySamples);
    if (flags.continentalness) this->continentalnessSamples = std::make_shared<Samples2d>(continentalnessSamples);
    if (flags.erosion) this->erosionSamples = std::make_shared<Samples2d>(erosionSamples);
    if (flags.weirdness) this->weirdnessSamples = std::make_shared<Samples2d>(weirdnessSamples);
    if (flags.pv) this->pvSamples = std::make_shared<Samples2d>(pvSamples);
    if (flags.offset) this->offsetSamples = std::make_shared<Samples2d>(offsetSamples);
    if (flags.depth) this->depthSamples = std::make_shared<Samples3d>(depthSamples);

    if (flags.biomes && !flags.biomeSurfaceOnly) {
        std::pair<Biome, std::unique_ptr<Biomes3d>> biomes = populateBiomes(temperatureSamples, humiditySamples, continentalnessSamples, erosionSamples, weirdnessSamples, depthSamples);

        this->biome = biomes.first;
        if (!biomes.first) {
            this->biomes = std::unique_ptr<Biomes3d>();
            std::get<std::unique_ptr<Biomes3d>>(this->biomes).swap(biomes.second);
        }
    } else if (flags.biomes) {
        std::pair<Biome, std::unique_ptr<Biomes2d>> biomes = populateFlatBiomes(temperatureSamples, humiditySamples, continentalnessSamples, erosionSamples, weirdnessSamples);

        this->biome = biomes.first;
        if (!biomes.first) {
            this->biomes = std::unique_ptr<Biomes2d>();
            std::get<std::unique_ptr<Biomes2d>>(this->biomes).swap(biomes.second);
        }
    } else this->biome = THE_VOID;
}