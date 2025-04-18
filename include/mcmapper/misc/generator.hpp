#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include <mcmapper/rng/noises.hpp>

#include <array>
#include <optional>

typedef std::array<f32, 16> Samples2d;
typedef std::array<std::array<f32, 96>, 16> Samples3d;
typedef std::array<Biome, 16> Biomes2d;
typedef std::array<std::array<Biome, 96>, 16> Biomes3d;

struct ChunkGeneratorFlags {
    bool biomes : 1 = false;
    bool biomeSurfaceOnly : 1 = false;

    bool temperature : 1 = false;
    bool humidity : 1 = false;
    bool continentalness : 1 = false;
    bool erosion : 1 = false;
    bool weirdness : 1 = false;
    bool offset : 1 = false;
    bool depth : 1 = false;
    bool pv : 1 = false;
};

struct ChunkGenerator {
    i32 x, z;

    std::shared_ptr<Samples2d>
        temperatureSamples,
        humiditySamples,
        continentalnessSamples,
        erosionSamples,
        weirdnessSamples,
        pvSamples,
        offsetSamples;
    std::shared_ptr<Samples3d>
        depthSamples;

    explicit ChunkGenerator(i32 chunkX, i32 chunkZ, std::shared_ptr<Noises> noises, const ChunkGeneratorFlags& flags) {
        this->x = chunkX;
        this->z = chunkZ;
        this->noises = noises;

        this->populate(flags);
    }

    // x, y, and z in biome coordinates (>> 2 if block)
    inline Biome getBiomeAt(i32 x, i32 y, i32 z) const {
        // Single biome chunk
        if (this->biome != THE_VOID) return this->biome;
        
        // Flat chunk
        if (std::holds_alternative<std::unique_ptr<Biomes2d>>(this->biomes)) {
            return std::get<std::unique_ptr<Biomes2d>>(this->biomes)->at(x + (z >> 2));
        }

        // 3D chunk
        return std::get<std::unique_ptr<Biomes3d>>(this->biomes)->at(x + (z >> 2)).at(y >> 2);
    }

private:
    void populate(const ChunkGeneratorFlags& flags);

    std::shared_ptr<Noises> noises;

    // Many chunks only hold a single biome.
    // If this chunk is one of them, we can save so much space this way.
    Biome biome;
    std::variant<std::unique_ptr<Biomes2d>, std::unique_ptr<Biomes3d>> biomes;
};

#endif