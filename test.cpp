#include <mcmapper/structure/structures.hpp>
#include <mcmapper/mcmapper.hpp>
#include <mcmapper/loot/builtin_loot.hpp>
#include <mcmapper/misc/generator.hpp>
#include <mcmapper/terrain/terrainextra.hpp>

#include <iostream>
#include <chrono>

void sampleColumn(TerrainGeneratorConfig& config, f64 x, f64 z) {
    for (int y = 256; y > 0; --y) {
        f64 newDensity = sampleFinalDensity(config, x, y, z);
        std::cout << "[" << y << "] " << newDensity << std::endl;
    }
}

void sampleNether(f64 x, f64 z, u64 worldSeed) {
    TerrainGeneratorConfig config(worldSeed, Dimension::DIM_NETHER);
    for (int y = 128; y > 0; --y) {
        double density = sampleDensityNether(config, x, y, z);
        std::cout << "[" << y << "] " << density << std::endl;
    }
}

void printBool(bool b) {
    if (b) std::cout << "true" << std::endl;
    else std::cout << "false" << std::endl;
}

//tests
//if these fail, that isn't good
//well it doesn't work anymore but whatever
int main() {
    const i64 worldSeed = 3447;

    sampleNether(0, 0, worldSeed);

    //std::chrono::high_resolution_clock clock;
    //auto start = clock.now();

    //std::shared_ptr<Noises> noises = std::make_shared<Noises>(worldSeed);

/*    ClimateNoises cnoises(worldSeed);

    std::array<Biome, 16> biomes;

    for (int i = 0; i < 16; ++i) {
        biomes[i] = cnoises.getBiomeAt(i - i % 4, 256, i % 4);
    }

    auto end = clock.now();
    std::cout << "Finished ClimateNoises test in " << (end - start).count() << "ns" << std::endl;

    for (int i = 0; i < 16; ++i) {
        std::cout << biomeRepr(biomes[i]) << std::endl;
    }

    start = clock.now();

    const ChunkGeneratorFlags flags = {.biomes=true, .biomeSurfaceOnly=true};
    ChunkGenerator generator(0, 0, noises, flags);

    end = clock.now();
    std::cout << "Finished ChunkGenerator test in " << (end - start).count() << "ns" << std::endl;

    for (int i = 0; i < 16; ++i) {
        std::cout << biomeRepr(generator.getBiomeAt(std::floor(i / 4), 256, i % 4)) << std::endl;
    }

/*
    std::cout << "Found village at " << placements::VILLAGE.findStart(worldSeed, 0, 0).chunkToBlock() << " in seed 3447; expected (48, 0)" << std::endl;

    ClimateNoises noises(worldSeed);

    std::chrono::high_resolution_clock clock;
    auto start = clock.now();
    Biome biome = noises.getBiomeAt(-686, 128, -1172);
    auto end = clock.now();
    std::cout << "Found biome " << biomeRepr(biome) << " at position -686, 128, -1172 in seed 3447; expected Pale Garden (time taken: " << end - start << ")" << std::endl;

    start = clock.now();
    biome = noises.getBiomeAt(0, 128, -512);
    end = clock.now();
    std::cout << "Found biome " << biomeRepr(biome) << " at position 0, 128, -512 in seed 3447; expected Ocean (time taken: " << end - start << ")" << std::endl;

    start = clock.now();
    biome = noises.getBiomeAt(-512, -48, 640);
    end = clock.now();
    std::cout << "Found biome " << biomeRepr(biome) << " at position -512, -48, 640 in seed 3447; expected Deep Dark (time taken: " << end - start << ")" << std::endl;

    start = clock.now();
    biome = noises.getBiomeAt(-896, -48, 1152);
    end = clock.now();
    std::cout << "Found biome " << biomeRepr(biome) << " at position -896, -48, 1152 in seed 3447; expected Deep Dark (time taken: " << end - start << ")" << std::endl;

/*
    int heightEstimate = noises.estimateHeight(-92, 327);
    std::cout << "Estimated surface at position (-92, 327) to be " << heightEstimate << ", expected about 70..." << std::endl;

    heightEstimate = noises.estimateHeight(0, -512);
    std::cout << "Estimated surface at position (0, -512) to be " << heightEstimate << ", expected < 64 (ocean biome)..." << std::endl;
    
    /-*
    heightEstimate = noises.estimateStoneHeight(-92, 327);
    std::cout << "Secondary estimate of surface height at position (-92, 327) is " << heightEstimate << "..." << std::endl;

    heightEstimate = noises.estimateStoneHeight(0, -512);
    std::cout << "Secondary estimate of surface height at position (0, -512) is " << heightEstimate << "..." << std::endl;

    sampleAll(noises, 0, -512);
    sampleAll(noises, -80, 320);

    int startX = -80, startZ = 320;
    for (int x = startX; x < startX + 16; ++x) for (int z = startZ; z < startZ + 16; ++z) {
        int estimate = noises.estimateStoneHeight(x, z);
        std::cout << "At (" << x << ", " << z << "): " << estimate << std::endl;
    }
///*-/

    auto logLoot = [](ItemStack stack) -> void {
        std::cout << stack.id << " x " << stack.count << std::endl;
        if (stack.enchantments.has_value())
            std::cout << "\t" << stack.enchantments.value()[0].id << " " << stack.enchantments.value()[0].level << std::endl;
    };

    std::shared_ptr<LootTable> pyramidTable = desertPyramidLoot();

    std::cout << "33333" << std::endl;
    LootContext context(33333);
    pyramidTable->roll(context, logLoot);

    std::cout << "7649212224873991995" << std::endl;
    context = LootContext(7649212224873991995LL);
    pyramidTable->roll(context, logLoot);

    std::cout << "-8167473274498177545" << std::endl;
    context = LootContext(-8167473274498177545LL);
    pyramidTable->roll(context, logLoot);//*/

    //sampleColumn(*noises, 92, 327);
    //sampleColumn(*noises, -768, 1024);

    //TerrainGeneratorConfig config(noises, worldSeed);
    //sampleColumn(config, 281, -233); // should be ~40

    return 0;
}