#include <mcmapper/structures.hpp>
#include <mcmapper/mcmapper.hpp>
#include <mcmapper/loot.hpp>
#include <mcmapper/builtin_loot.hpp>

#include <iostream>
#include <chrono>

void sampleAll(ClimateNoises noises, int x, int z) {
    for (int i = 320; i > -64; --i) {
        f32 initialDensity = noises.sampleInitialDensity(x, i, z);
        std::cout << i << ": " << initialDensity << std::endl;
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

    ClimateNoises noises(worldSeed);

/*
    std::cout << "Found village at " << placements::VILLAGE.findStart(worldSeed, 0, 0).chunkToBlock() << " in seed 3447; expected (48, 0)" << std::endl;
*-/

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

*/
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

    /-*int startX = -80, startZ = 320;
    for (int x = startX; x < startX + 16; ++x) for (int z = startZ; z < startZ + 16; ++z) {
        int estimate = noises.estimateStoneHeight(x, z);
        std::cout << "At (" << x << ", " << z << "): " << estimate << std::endl;
    }*/
//*/

    auto logLoot = [](ItemStack stack) -> void {
        std::cout << stack.id << " x " << stack.count << std::endl;
        if (stack.enchantments.has_value())
            std::cout << "\t" << stack.enchantments.value()[0].id << " " << stack.enchantments.value()[0].level << std::endl;
    };

    std::shared_ptr<LootTable> pyramidTable = getOrBuildDesertPyramidLootTable();

    std::cout << "33333" << std::endl;
    LootContext context(33333);
    pyramidTable->roll(context, logLoot);

    std::cout << "7649212224873991995" << std::endl;
    context = LootContext(7649212224873991995LL);
    pyramidTable->roll(context, logLoot);

    std::cout << "-8167473274498177545" << std::endl;
    context = LootContext(-8167473274498177545LL);
    pyramidTable->roll(context, logLoot);

    return 0;
}