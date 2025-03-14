#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <set>

#include <mcmapper/mcmapper.hpp>
#include <mcmapper/structure/structure.hpp>

const i64 SEED = 684512;
const StructurePlacement EVOKER_FORT(85, 50, SpreadType::LINEAR, SEED);
const Tag<Biome> EVOKER_FORT_BIOMES = Tag<Biome>({Biome::WINDSWEPT_SAVANNA, Biome::SAVANNA, Biome::SAVANNA_PLATEAU}).add(Biome::DESERT).add({Biome::PLAINS, Biome::SUNFLOWER_PLAINS}).add(biometags::HILL).add(biometags::FOREST);

// Demonstration of modded structures
// (this is from a mod)
int main() {
    std::fstream file("out.log", file.trunc | file.in | file.out);
    std::set<Pos2D> positions;
    ClimateNoises noises(SEED);

    for (int i = -8; i < 8; ++i) for (int j = -8; j < 8; ++j) {
        Pos2D localStartPos = EVOKER_FORT.findStart(SEED, i, j).chunkToBlock();
        Biome localBiome = noises.getBiomeAt(localStartPos.x, 128, localStartPos.z);
        if (EVOKER_FORT_BIOMES.contains(localBiome)) positions.insert(localStartPos);
    }

    for (Pos2D startPos : positions) {
        file << startPos.toString() << std::endl;
    }

    return 0;
}