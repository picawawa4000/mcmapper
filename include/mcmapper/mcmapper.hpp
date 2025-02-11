#ifndef MCMAPPER_HPP
#define MCMAPPER_HPP

#include <cmath>
#include <mcmapper/noises.hpp>
#include <mcmapper/biomes.hpp>

bool isSlimeChunk(i64 worldSeed, i32 chunkX, i32 chunkZ) {
    i64 i = worldSeed;
    i += (int)(chunkX * 0x5ac0db);
    i += (int)(chunkX * chunkX * 0x4c1906);
    i += (int)(chunkZ * 0x5f24f);
    i += (int)(chunkZ * chunkZ) * 0x4307a7ULL;
    i ^= 0x3ad8025fULL;
    return CheckedRandom(i).next_i32(10) == 0;
}

#endif