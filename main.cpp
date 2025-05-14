#include <mcmapper/terrain/terrain.hpp>

#include <sstream>
#include <iostream>
#include <fstream>

void printHeightmap(TerrainGeneratorConfig& config, i32 chunkX, i32 chunkZ, std::ostream& out) {
    i32 startX = chunkX * 16;
    i32 startZ = chunkZ * 16;
    for (int x = startX; x < startX + 16; ++x) {
        for (int z = startZ; z < startZ + 16;) {
            for (int y = 256; y > -64; --y) {
                if (sampleFinalDensity(config, x, y, z) >= 0) {
                    out << "[" << x << ", " << z << "] " << y << std::endl;
                    goto COLUMN_HEIGHT_FOUND;
                }
            }
            out << "[" << x << ", " << z << "] N/A" << std::endl;
            COLUMN_HEIGHT_FOUND: ++z;
        }
    }
}

int main() {
    const u64 worldSeed = 3447;
    TerrainGeneratorConfig config(worldSeed, Dimension::DIM_OVERWORLD);
    printHeightmap(config, 0, 0, std::cout);

    return 0;
}