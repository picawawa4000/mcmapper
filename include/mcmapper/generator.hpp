#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include <mcmapper/noises.hpp>

#include <array>
#include <optional>

struct ChunkGenerator {
    i32 x, z;

    explicit ChunkGenerator(i32 chunkX, i32 chunkZ, std::shared_ptr<Noises> noises) {
        this->x = chunkX;
        this->z = chunkZ;
        this->noises = noises;

        this->populate();
    }

    // x, y, and z in biome coordinates (>> 2 if block)
    Biome getBiomeAt(i32 x, i32 y, i32 z) {
        if (this->biome != THE_VOID) return this->biome;
        return this->biomes->at(x + z >> 2).at(y >> 2);
    }

private:
    void populate();

    std::shared_ptr<Noises> noises;

    // Many chunks only hold a single biome.
    // If this chunk is one of them, we can save so much space this way.
    Biome biome;
    std::unique_ptr<std::array<std::array<Biome, 96>, 16>> biomes;
};

#endif