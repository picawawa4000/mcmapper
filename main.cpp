#include <mcmapper/terrain/terrain.hpp>

#include <sstream>
#include <iostream>
#include <fstream>

void printHeightmap(TerrainGeneratorConfig& config, i32 fromX, i32 fromZ, i32 toX, i32 toZ, std::ostream& out) {
    for (int x = fromX; x < toX; ++x) {
        for (int z = fromZ; z < toZ;) {
            for (int y = 256; y > -64; --y) {
                if (sampleFinalDensity(config, x, y, z) >= 0.) {
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
    u64 worldSeed = 3447ULL;

    TerrainGeneratorConfig config(worldSeed, Dimension::DIM_OVERWORLD);
    std::ofstream log("surface_heights.log");

    printHeightmap(config, 44, 0, 49, 9, log);

    log.close();

    return 0;
}