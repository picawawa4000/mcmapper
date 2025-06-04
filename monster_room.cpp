#include <mcmapper/terrain/terrain.hpp>

bool isSolid(TerrainGeneratorConfig& config, i32 x, i32 y, i32 z) {
    return sampleFinalDensity(config, x, y, z) > 0.;
}

bool canGenerateDungeon(TerrainGeneratorConfig& config, XoroshiroRandom& rng, i32 x, i32 y, i32 z) {
    i32 xSize = rng.next_i32(2) + 3;
    i32 zSize = rng.next_i32(2) + 3;

    i32 lowerX = x - xSize;
    i32 upperX = x + xSize;
    i32 lowerZ = z - zSize;
    i32 upperZ = z + zSize;

    i32 openings = 0;

    for (i32 trialX = lowerX; trialX <= upperX; ++trialX) {
        for (i32 trialZ = lowerZ; trialZ <= upperZ; ++trialZ) {
            if (trialX != lowerX && trialX != upperX && trialZ != lowerZ && trialZ != upperZ) continue;
            // This can probably be unrolled, but that's for later.
            for (i32 trialYOffset = -1; trialYOffset <= 4; ++trialYOffset) {
                if (trialYOffset == 1 || trialYOffset == 2 || trialYOffset == 3) continue;

                i32 trialY = trialYOffset + y;

                bool solid = isSolid(config, trialX, trialY, trialZ);
                if ((trialYOffset == -1 || trialYOffset == 4) && !solid) return false;

                if (trialYOffset != 0) continue;

                if (!solid || !isSolid(config, trialX, trialY + 1, trialZ)) continue;
                ++openings;
            }
        }
    }
    return openings >= 1 && openings <= 5;
}

int main() {
    const u64 seed = 7059816948748491006ULL;
    const i32 chunkX = -3, chunkZ = -1;
    const i32 blockX = chunkX * 16, blockZ = chunkZ * 16;

    TerrainGeneratorConfig config(seed, Dimension::DIM_OVERWORLD);
    
    /// TODO: turn all of this into a separate function
    XoroshiroRandom rng(seed);
    u64 useBlockX = rng.next_u64() | 1L;
    u64 useBlockZ = rng.next_u64() | 1L;
    u64 populationSeed = (blockX * useBlockX + blockZ * useBlockZ) ^ seed;
    u64 decoratorSeed = populationSeed + 3 /* index */ + (10000 * 3 /* step */);
    rng.setSeed(decoratorSeed);

    std::array<Pos3D, 4> attemptPositions = {Pos3D{blockX, -64, blockZ}, {blockX, -64, blockZ}, {blockX, -64, blockZ}, {blockX, -64, blockZ}};
    for (int i = 0; i < 4; ++i) {
        attemptPositions[i].x += rng.next_i32(16);
        attemptPositions[i].z += rng.next_i32(16);
        attemptPositions[i].y = rng.next_i32(58) - 58;
    }

    std::cout << "Attempting at positions " << attemptPositions[0].toString() << ", " << attemptPositions[1].toString() << ", " << attemptPositions[2].toString() << ", " << attemptPositions[3].toString() << std::endl;

    for (int i = 0; i < 4; ++i) {
        if (canGenerateDungeon(config, rng, attemptPositions[i].x, attemptPositions[i].y, attemptPositions[i].z)) {
            std::cout << "Generated dungeon at position " << i << std::endl;
        }
    }

    return 0;
}