#include <mcmapper/rng/noises.hpp>

namespace data {
    std::vector<std::vector<u64>> hashtable = {
        {0x5c7e6b29735f0d7f, 0xf7d86f1bbc734988}, //"minecraft:temperature"
        {0x81bb4d22e8dc168e, 0xf1c8b4bea16303cd}, //"minecraft:vegetation"
        {0x83886c9d0ae3a662, 0xafa638a61b42e8ad}, //"minecraft:continentalness"
        {0xd02491e6058f6fd8, 0x4792512c94c17a80}, //"minecraft:erosion"
        {0xefc8ef4d36102b34, 0x1beeeb324a0f24ea}, //"minecraft:ridge"
        {0x080518cf6af25384, 0x3f3dfb40a54febd5}, //"minecraft:offset"
        {0x4b3097bbe1a7e1ac, 0xf49561571ec10de8}, //"minecraft:surface"
    };

    std::vector<f64> temperature_amplitudes = {1.5, 0., 1., 0., 0., 0.};
    std::vector<f64> humidity_amplitudes = {1., 1., 0., 0., 0., 0.};
    std::vector<f64> continentalness_amplitudes = {1., 1., 2., 2., 2., 1., 1., 1., 1.};
    std::vector<f64> erosion_amplitudes = {1., 1., 0., 1., 1.};
    std::vector<f64> weirdness_amplitudes = {1., 2., 1., 0., 0., 0.};
    std::vector<f64> offset_amplitudes = {1., 1., 1., 0.};
    std::vector<f64> surface_amplitudes = {1., 1., 1.};
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
}
