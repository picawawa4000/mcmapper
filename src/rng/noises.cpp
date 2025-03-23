#include <mcmapper/rng/noises.hpp>

//#include <iostream>
//#include <chrono>

namespace data {
    const std::array<const std::array<const u64, 2>, 23> hashtable = {{
        {0x5c7e6b29735f0d7f, 0xf7d86f1bbc734988}, //"minecraft:temperature"
        {0x81bb4d22e8dc168e, 0xf1c8b4bea16303cd}, //"minecraft:vegetation"
        {0x83886c9d0ae3a662, 0xafa638a61b42e8ad}, //"minecraft:continentalness"
        {0xd02491e6058f6fd8, 0x4792512c94c17a80}, //"minecraft:erosion"
        {0xefc8ef4d36102b34, 0x1beeeb324a0f24ea}, //"minecraft:ridge"
        {0x080518cf6af25384, 0x3f3dfb40a54febd5}, //"minecraft:offset"
        {0x4b3097bbe1a7e1ac, 0xf49561571ec10de8}, //"minecraft:surface"
        {0xa5053f53ce3f5840, 0x834fa38a3ded87b7}, //"minecraft:spaghetti_3d_1"
        {0xb9a6367335a0ceef, 0xd61ccfd0fac10ac3}, //"minecraft:spaghetti_3d_2"
        {0xee4780c98d93a439, 0xbe45d334fdb76d2c}, //"minecraft:spaghetti_3d_rarity"
        {0x897e1f20ad2d2b27, 0xf4142f5a58d1d5ab}, //"minecraft:spaghetti_3d_thickness"
        {0xf1008a17493d9a65, 0xbd1ce09e8bc70ea0}, //"minecraft:cave_entrance"
        {0x7aed57fd327d6591, 0xd495a3593e4d67bd}, //"minecraft:spaghetti_roughness"
        {0xe19387d2ceaf4eaa, 0xcacdcd34e1bc2003}, //"minecraft:spaghetti_roughness_modulator"
        {0x4dfe67be2ef51a83, 0x5a4ae8c4423e7206}, //"minecraft:cave_layer"
        {0xb159093bc7baaa50, 0x53abc45424417c20}, //"minecraft:cave_cheese"
        {0x34748c98fa19c477, 0x2a02051eb9e9b9cd}, //"minecraft:spaghetti_2d"
        {0x95e07097e4685522, 0x0232deaf95eb5fed}, //"minecraft:spaghetti_2d_modulator"
        {0x29dd5fcf38d9edcd, 0x5bb1d7a839f6d4ab}, //"minecraft:spaghetti_2d_elevation"
        {0x37e5cb432b85f4c8, 0xe5e16b35b407aebc}, //"minecraft:spaghetti_2d_thickness"
        {0xd4defd1400fa5347, 0xccb6785451feaa1c}, //"minecraft:pillar"
        {0x8ddd6be7cd4b24d0, 0x0485e8665333197a}, //"minecraft:pillar_rareness"
        {0x1a28cb2542f8308d, 0xc4bba10b7fc7168c}, //"minecraft:pillar_thickness"
    }};

    const std::vector<f64> temperature_amplitudes = {1.5, 0., 1., 0., 0., 0.};
    const std::vector<f64> humidity_amplitudes = {1., 1., 0., 0., 0., 0.};
    const std::vector<f64> continentalness_amplitudes = {1., 1., 2., 2., 2., 1., 1., 1., 1.};
    const std::vector<f64> erosion_amplitudes = {1., 1., 0., 1., 1.};
    const std::vector<f64> weirdness_amplitudes = {1., 2., 1., 0., 0., 0.};
    const std::vector<f64> offset_amplitudes = {1., 1., 1., 0.};
    const std::vector<f64> surface_amplitudes = {1., 1., 1.};
    const std::vector<f64> cave_entrance_amplitudes = {0.4, 0.5, 1.};
    const std::vector<f64> cave_cheese_amplitudes = {0.5, 1., 2., 1., 2., 1., 0., 2., 2.};
};

ClimateNoises::ClimateNoises(i64 worldSeed) {
    XoroshiroRandom random((u64)worldSeed);
    u64 lo = random.next_u64();
    u64 hi = random.next_u64();

    XoroshiroRandom temperature_random(lo ^ data::hashtable[0][0], hi ^ data::hashtable[0][1]);
    XoroshiroRandom humidity_random(lo ^ data::hashtable[1][0], hi ^ data::hashtable[1][1]);
    XoroshiroRandom continentalness_random(lo ^ data::hashtable[2][0], hi ^ data::hashtable[2][1]);
    XoroshiroRandom erosion_random(lo ^ data::hashtable[3][0], hi ^ data::hashtable[3][1]);
    XoroshiroRandom weirdness_random(lo ^ data::hashtable[4][0], hi ^ data::hashtable[4][1]);
    XoroshiroRandom offset_random(lo ^ data::hashtable[5][0], hi ^ data::hashtable[5][1]);
    
    XoroshiroRandom surface_random(lo ^ data::hashtable[6][0], hi ^ data::hashtable[6][1]);

    this->temperature = DoublePerlinNoise(temperature_random, data::temperature_amplitudes, -10);
    this->humidity = DoublePerlinNoise(humidity_random, data::humidity_amplitudes, -8);
    this->continentalness = DoublePerlinNoise(continentalness_random, data::continentalness_amplitudes, -9);
    this->erosion = DoublePerlinNoise(erosion_random, data::erosion_amplitudes, -9);
    this->weirdness = DoublePerlinNoise(weirdness_random, data::weirdness_amplitudes, -7);
    this->offset = DoublePerlinNoise(offset_random, data::offset_amplitudes, -3);

    this->surface = DoublePerlinNoise(surface_random, data::surface_amplitudes, -6);

    this->seedLo = lo;
    this->seedHi = hi;
}

Noises::Noises(i64 worldSeed) {
//    std::chrono::high_resolution_clock clock;
//    auto start = clock.now();

    XoroshiroRandom rng(worldSeed);
    u64 lo = rng.next_u64();
    u64 hi = rng.next_u64();

    this->seedLo = lo;
    this->seedHi = hi;

    XoroshiroRandom temp(lo ^ data::hashtable[0][0], hi ^ data::hashtable[0][1]);
    this->temperature = DoublePerlinNoise(temp, data::temperature_amplitudes, -10);

    temp = XoroshiroRandom(lo ^ data::hashtable[1][0], hi ^ data::hashtable[1][1]);
    this->humidity = DoublePerlinNoise(temp, data::humidity_amplitudes, -8);

    temp = XoroshiroRandom(lo ^ data::hashtable[2][0], hi ^ data::hashtable[2][1]);
    this->continentalness = DoublePerlinNoise(temp, data::continentalness_amplitudes, -9);

    temp = XoroshiroRandom(lo ^ data::hashtable[3][0], hi ^ data::hashtable[3][1]);
    this->erosion = DoublePerlinNoise(temp, data::erosion_amplitudes, -9);

    temp = XoroshiroRandom(lo ^ data::hashtable[4][0], hi ^ data::hashtable[4][1]);
    this->weirdness = DoublePerlinNoise(temp, data::weirdness_amplitudes, -7);

    temp = XoroshiroRandom(lo ^ data::hashtable[5][0], hi ^ data::hashtable[5][1]);
    this->offset = DoublePerlinNoise(temp, data::offset_amplitudes, -3);

    temp = XoroshiroRandom(lo ^ data::hashtable[6][0], hi ^ data::hashtable[6][1]);
    this->surface = DoublePerlinNoise(temp, data::surface_amplitudes, -6);

    temp = XoroshiroRandom(lo ^ data::hashtable[7][0], hi ^ data::hashtable[7][1]);
    this->spaghetti3d_1 = DoublePerlinNoise(temp, {1.}, -7);

    temp = XoroshiroRandom(lo ^ data::hashtable[8][0], hi ^ data::hashtable[8][1]);
    this->spaghetti3d_2 = DoublePerlinNoise(temp, {1.}, -7);

    temp = XoroshiroRandom(lo ^ data::hashtable[9][0], hi ^ data::hashtable[9][1]);
    this->spaghetti3d_rarity = DoublePerlinNoise(temp, {1.}, -11);

    temp = XoroshiroRandom(lo ^ data::hashtable[10][0], hi ^ data::hashtable[10][1]);
    this->spaghetti3d_thickness = DoublePerlinNoise(temp, {1.}, -8);

    temp = XoroshiroRandom(lo ^ data::hashtable[11][0], hi ^ data::hashtable[11][1]);
    this->cave_entrance = DoublePerlinNoise(temp, data::cave_entrance_amplitudes, -7);

    temp = XoroshiroRandom(lo ^ data::hashtable[12][0], hi ^ data::hashtable[12][1]);
    this->spaghetti_roughness = DoublePerlinNoise(temp, {1.}, -5);

    temp = XoroshiroRandom(lo ^ data::hashtable[13][0], hi ^ data::hashtable[13][1]);
    this->spaghetti_roughness_modulator = DoublePerlinNoise(temp, {1.}, -8);

    temp = XoroshiroRandom(lo ^ data::hashtable[14][0], hi ^ data::hashtable[14][1]);
    this->cave_layer = DoublePerlinNoise(temp, {1.}, -8);

    temp = XoroshiroRandom(lo ^ data::hashtable[15][0], hi ^ data::hashtable[15][1]);
    this->cave_cheese = DoublePerlinNoise(temp, data::cave_cheese_amplitudes, -8);

    temp = XoroshiroRandom(lo ^ data::hashtable[16][0], hi ^ data::hashtable[16][1]);
    this->spaghetti2d = DoublePerlinNoise(temp, {1.}, -7);

    temp = XoroshiroRandom(lo ^ data::hashtable[17][0], hi ^ data::hashtable[17][1]);
    this->spaghetti2d_modulator = DoublePerlinNoise(temp, {1.}, -11);

    temp = XoroshiroRandom(lo ^ data::hashtable[18][0], hi ^ data::hashtable[18][1]);
    this->spaghetti2d_elevation = DoublePerlinNoise(temp, {1.}, -8);

    temp = XoroshiroRandom(lo ^ data::hashtable[19][0], hi ^ data::hashtable[19][1]);
    this->spaghetti2d_thickness = DoublePerlinNoise(temp, {1.}, -11);

    temp = XoroshiroRandom(lo ^ data::hashtable[20][0], hi ^ data::hashtable[20][1]);
    this->pillar = DoublePerlinNoise(temp, {1., 1.}, -7);

    temp = XoroshiroRandom(lo ^ data::hashtable[21][0], hi ^ data::hashtable[21][1]);
    this->pillar_rareness = DoublePerlinNoise(temp, {1.}, -8);

    temp = XoroshiroRandom(lo ^ data::hashtable[22][0], hi ^ data::hashtable[22][1]);
    this->pillar_thickness = DoublePerlinNoise(temp, {1.}, -8);

//    auto end = clock.now();
//    std::cout << "Creating noises took " << end - start << std::endl;
}
