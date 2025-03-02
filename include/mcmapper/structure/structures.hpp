#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

#include <mcmapper/structure/structure.hpp>

namespace placements {
    const StructurePlacement VILLAGE(34, 8, SpreadType::LINEAR, 10387312);
    const StructurePlacement DESERT_PYRAMID(32, 8, SpreadType::LINEAR, 14357617);
    const StructurePlacement IGLOO(32, 8, SpreadType::LINEAR, 14357618);
    const StructurePlacement JUNGLE_TEMPLE(32, 8, SpreadType::LINEAR, 14357619);
    const StructurePlacement SWAMP_HUT(32, 8, SpreadType::LINEAR, 14357620);
    const StructurePlacement PILLAGER_OUTPOST(32, 8, SpreadType::LINEAR, 165745296);
    const StructurePlacement ANCIENT_CITY(24, 8, SpreadType::LINEAR, 20083232);
    const StructurePlacement OCEAN_MONUMENT(32, 5, SpreadType::TRIANGULAR, 10387313);
    const StructurePlacement WOODLAND_MANSION(80, 20, SpreadType::TRIANGULAR, 10387319);
    const StructurePlacement MINESHAFT(1, 0, SpreadType::LINEAR, 0);
    const StructurePlacement RUINED_PORTAL(40, 15, SpreadType::LINEAR, 34222645);
    const StructurePlacement SHIPWRECK(24, 4, SpreadType::LINEAR, 165745295);
    const StructurePlacement OCEAN_RUIN(20, 8, SpreadType::LINEAR, 14357621);
    const StructurePlacement NETHER_STRUCTURE(27, 4, SpreadType::LINEAR, 30084232);
    const StructurePlacement NETHER_FOSSIL(2, 1, SpreadType::LINEAR, 14357921);
    const StructurePlacement END_CITY(20, 11, SpreadType::TRIANGULAR, 10387313);
    const StructurePlacement TRAIL_RUIN(34, 8, SpreadType::LINEAR, 83469867);
    const StructurePlacement TRIAL_CHAMBER(32, 8, SpreadType::LINEAR, 94251327);
}

// Buried treasures work differently from other structures.
// They have a 1 in 100 chance to spawn in every chunk.
bool checkBuriedTreasure(i64 worldSeed, i64 chunkX, i64 chunkZ) {
    i64 blockX = chunkX * 16 + 9, blockZ = chunkZ * 16 + 9;
    CheckedRandom rng(chunkX * 341873128712LL + chunkZ * 132897987541LL + worldSeed + 10387320LL);
    return rng.next_f32() < 0.01;
}

namespace structures {
    
}

#endif