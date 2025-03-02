#include <mcmapper/rng/rng.hpp>

bool isSlimeChunk(i64 worldSeed, i32 chunkX, i32 chunkZ) {
    i64 i = worldSeed;
    i += (int)(chunkX * 0x5ac0db);
    i += (int)(chunkX * chunkX * 0x4c1906);
    i += (int)(chunkZ * 0x5f24f);
    i += (int)(chunkZ * chunkZ) * 0x4307a7ULL;
    i ^= 0x3ad8025fULL;
    return CheckedRandom(i).next_i32(10) == 0;
}

XoroshiroRandom split(i64 seedLo, i64 seedHi, i32 blockX, i32 blockY, i32 blockZ) {
    i64 hash = hashCode(blockX, blockY, blockZ) ^ seedLo;
    return XoroshiroRandom(hash, seedHi);
}
