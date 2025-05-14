#include <mcmapper/rng/noise.hpp>

#include <iostream>
#include <fstream>

enum OreVeinBlock {
    NONE = 0,
    RAW_COPPER_BLOCK,
    COPPER_ORE,
    GRANITE,
    RAW_IRON_BLOCK,
    IRON_ORE,
    TUFF,
};

// minecraft:ore
const u64 UPPER_ORE_HASH = 0x9b88124de600116dULL;
const u64 LOWER_ORE_HASH = 0x2ae68055aa4a7761ULL;

// minecraft:ore_vein_a
const u64 UPPER_ORE_VEIN_A_HASH = 0x4cd8d69b9a841649ULL;
const u64 LOWER_ORE_VEIN_A_HASH = 0xcdd63f17bfe8f5edULL;

// minecraft:ore_vein_b
const u64 UPPER_ORE_VEIN_B_HASH = 0x6b26220b31f7c6c9ULL;
const u64 LOWER_ORE_VEIN_B_HASH = 0xae077edebf6aaec1ULL;

// minecraft:ore_veininess
const u64 UPPER_ORE_VEININESS_HASH = 0x6b86c7820a307171ULL;
const u64 LOWER_ORE_VEININESS_HASH = 0xd87fb0fefd9c1624ULL;

// minecraft:ore_gap
const u64 UPPER_ORE_GAP_HASH = 0x9c4cc6b2fb0be4bbULL;
const u64 LOWER_ORE_GAP_HASH = 0xbd5964705573bb5eULL;

struct OreVeinParams {
    DoublePerlinNoise * veininess, * veinA, * veinB, * gap;
    u64 splitterHi, splitterLo;
    std::ofstream * log;
};

void logParams(i32 x, i32 y, i32 z, OreVeinParams& params) {
    *params.log << "[" << x << ", " << y << ", " << z << "] ";

    double veinToggle = params.veininess->sample(1.5 * x, 1.5 * y, 1.5 * z);
    *params.log << "Vein Toggle: " << veinToggle;

    double veinRidgedA = std::abs(params.veinA->sample(x * 4., y * 4., z * 4.));
    double veinRidgedB = std::abs(params.veinB->sample(x * 4., y * 4., z * 4.));
    double veinRidged = std::max(veinRidgedA, veinRidgedB) - 0.08f;
    *params.log << ", Vein Ridged: " << veinRidged << " (A: " << veinRidgedA << ", B: " << veinRidgedB;

    double veinGap = params.gap->sample(x, y, z);
    *params.log << "), Vein Gap: " << veinGap;

    float positionalRandom = split(params.splitterLo, params.splitterHi, x, y, z).next_f32();
    *params.log << ", Positional Random: " << positionalRandom << std::endl;
}

OreVeinBlock getOreVeinBlockAt(i32 x, i32 y, i32 z, OreVeinParams& params) {
    if (-60 > y || y > 50) return NONE;

    // skipping Y checking here because Y is guaranteed to be between -60 and 50
    if (std::abs(params.veinA->sample(x * 4., y * 4., z * 4.)) >= 0.08f || std::abs(params.veinB->sample(x * 4., y * 4., z * 4.)) >= 0.08f) return NONE;

    double veinToggle = params.veininess->sample(1.5 * x, 1.5 * y, 1.5 * z);
    double absVeinToggle = std::abs(veinToggle);
    i32 veinTypeTopY = veinToggle > 0. ? 50 : -8;
    i32 veinTypeBottomY = veinToggle > 0. ? 0 : -60;
    i32 belowTop = veinTypeTopY - y;
    i32 aboveBottom = y - veinTypeBottomY;
    if (belowTop < 0 || aboveBottom < 0) return NONE;

    i32 distanceToEdge = std::min(belowTop, aboveBottom);
    double weight = clampedLerp(-.2, 0., getLerpProgress(distanceToEdge, 0., 20.));
    if (absVeinToggle + weight < .4f) return NONE;

    XoroshiroRandom rng = split(params.splitterLo, params.splitterHi, x, y, z);
    if (rng.next_f32() > .7f) return NONE;

    double g = clampedLerp(.1f, .3f, getLerpProgress(distanceToEdge, .4f, .6f));
    if (rng.next_f32() < g && params.gap->sample(x, y, z) > -.3f) {
        if (rng.next_f32() < .02f) return veinToggle > 0. ? RAW_COPPER_BLOCK : RAW_IRON_BLOCK;
        return veinToggle > 0. ? COPPER_ORE : IRON_ORE;
    }
    return veinToggle > 0. ? GRANITE : TUFF;
}

int main() {
    const u64 worldSeed = 8052710360952744907ULL;
    const Pos2D chunkPos = {0, 0};

    XoroshiroRandom baseRng(worldSeed);
    u64 lo = baseRng.next_u64();
    u64 hi = baseRng.next_u64();

    XoroshiroRandom rOreVeinA(lo ^ UPPER_ORE_VEIN_A_HASH, hi ^ LOWER_ORE_VEIN_A_HASH);
    DoublePerlinNoise oreVeinA(rOreVeinA, {1.}, -7);

    XoroshiroRandom rOreVeinB(lo ^ UPPER_ORE_VEIN_B_HASH, hi ^ LOWER_ORE_VEIN_B_HASH);
    DoublePerlinNoise oreVeinB(rOreVeinB, {1.}, -7);

    XoroshiroRandom rOreVeininess(lo ^ UPPER_ORE_VEININESS_HASH, hi ^ LOWER_ORE_VEININESS_HASH);
    DoublePerlinNoise oreVeininess(rOreVeininess, {1.}, -8);

    XoroshiroRandom rOreGap(lo ^ UPPER_ORE_GAP_HASH, hi ^ LOWER_ORE_GAP_HASH);
    DoublePerlinNoise oreGap(rOreGap, {1.}, -5);

    XoroshiroRandom rPositionalOre(lo ^ UPPER_ORE_HASH, hi ^ LOWER_ORE_HASH);
    u64 oreLo = rPositionalOre.next_u64();
    u64 oreHi = rPositionalOre.next_u64();
    
    std::ofstream log("veinToggleLog.log");
    OreVeinParams params{.veinA=&oreVeinA, .veinB=&oreVeinB, .veininess=&oreVeininess, .gap=&oreGap, .splitterLo=oreLo, .splitterHi=oreHi, .log=&log};

    for (int x = -32; x < 32; ++x) {
        for (int z = -32; z < 32; ++z) {
            for (int y = -60; y < 50; ++y) {
                logParams(x, y, z, params);
                OreVeinBlock block = getOreVeinBlockAt(x, y, z, params);
                if (!block) continue;
                
                std::cout << "[" << x << ", " << y << ", " << z << "] ";
                switch (block) {
                case RAW_COPPER_BLOCK:
                    std::cout << "Raw copper block!!" << std::endl;
                    break;
                case RAW_IRON_BLOCK:
                    std::cout << "Raw iron block!!" << std::endl;
                    break;
                case COPPER_ORE:
                    std::cout << "Copper ore!" << std::endl;
                    break;
                case IRON_ORE:
                    std::cout << "Iron ore!" << std::endl;
                    break;
                case GRANITE:
                    std::cout << "Granite" << std::endl;
                    break;
                case TUFF:
                    std::cout << "Tuff" << std::endl;
                    break;
                default: // unreachable
                    break;
                }
            }
        }
    }

    log.flush();
    log.close();

    return 0;
}