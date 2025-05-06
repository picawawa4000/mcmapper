#include <mcmapper/terrain/terrain.hpp>

#include <sstream>
#include <iostream>
#include <fstream>

int main() {
    const u64 worldSeed = 3447;

    TerrainGeneratorConfig config(worldSeed, Dimension::DIM_OVERWORLD);
    
    for (int y = 256; y > 0; --y) {
        double density = sampleInitialDensity(config, 32, y, 149);
        std::cout << "[32, " << y << ", 149] " << density << std::endl;
    }

    return 0;
}