#ifndef NOISES_HPP
#define NOISES_HPP

#include <mcmapper/rng/noise.hpp>
#include <mcmapper/biome/biometree.hpp>
#include <mcmapper/terrain/spline.hpp>

#include <numeric>
#include <iostream>

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

//for now, no large biomes
//I may eventually get around to adding those...
struct ClimateNoises {
    DoublePerlinNoise temperature;
    DoublePerlinNoise humidity;
    DoublePerlinNoise continentalness;
    DoublePerlinNoise erosion;
    DoublePerlinNoise weirdness;
    DoublePerlinNoise offset;

    DoublePerlinNoise surface;

    const Spline offsetSpline = createOffsetSpline();
    const Spline factorSpline = createFactorSpline();
    
    const std::shared_ptr<SearchTree> biomeSearchTree = getSearchTree();

    u64 seedLo, seedHi;

    ClimateNoises(i64 worldSeed) {
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

    inline f32 shiftX(f32 x, f32 z) const {
        return this->offset.sample(x, 0, z) * 4.;
    }

    inline f32 shiftZ(f32 x, f32 z) const {
        return this->offset.sample(z, x, 0) * 4.;
    }

    NoisePoint sample(int x, int y, int z, bool biome_scale = false, bool no_shift = false, bool no_depth = false) const {
        f32 inX = biome_scale ? x : (x >> 2), inY = biome_scale ? y : (y >> 2), inZ = biome_scale ? z : (z >> 2);
        if (!no_shift) {
            inX += this->shiftX(inX, inZ);
            inZ += this->shiftZ(inX, inZ);
        }

        f32 temperatureOut = (f32)(this->temperature.sample(inX, 0, inZ));
        f32 humidityOut = (f32)(this->humidity.sample(inX, 0, inZ));
        f32 continentalnessOut = (f32)(this->continentalness.sample(inX, 0, inZ));
        f32 erosionOut = (f32)(this->erosion.sample(inX, 0, inZ));
        f32 weirdnessOut = (f32)(this->weirdness.sample(inX, 0, inZ));

        f32 depth = 0.f;
        if (!no_depth) {
            std::array<f32, 4> params = {continentalnessOut, erosionOut, static_cast<f32>(pvTransform(weirdnessOut)), weirdnessOut};
            f32 depthOffset = ClimateNoises::offsetSpline.sample(params) - 0.50375f;
            depth = yClampedGradient(inY * 4., -64, 320, 1.5, -1.5) + depthOffset;
        }
        
        return NoisePoint(temperatureOut * 10000.f, humidityOut * 10000.f, continentalnessOut * 10000.f, erosionOut * 10000.f, depth * 10000.f, weirdnessOut * 10000.f);
    }

    //api note: if not using the pale garden, it can just be unconditionally replaced with dark forest for accurate results
    Biome getBiomeAt(int x, int y, int z, bool no_shift = false, bool no_depth = false) {
        NoisePoint point = this->sample(x, y, z, false, no_shift, no_depth);
        return this->biomeSearchTree->get(point);
    }

    // turns out that this is AS in the debug screen
    // this is correct btw
    f32 sampleInitialDensity(int x, int y, int z, bool biome_scale = false) const {
        f32 inX = (x >> 2) + this->shiftX(x, z);
        f32 inZ = (z >> 2) + this->shiftZ(x, z);

        f32 continentalnessOut = this->continentalness.sample(inX, 0, inZ);
        f32 erosionOut = this->erosion.sample(inX, 0, inZ);
        f32 weirdnessOut = this->weirdness.sample(inX, 0, inZ);
        f32 pvOut = pvTransform(weirdnessOut);

        std::array<f32, 4> values = {continentalnessOut, erosionOut, pvOut, weirdnessOut};

        f32 depthOffset = ClimateNoises::offsetSpline.sample(values) - 0.50375f;
        f32 depth = yClampedGradient(y, -64, 320, 1.5, -1.5) + depthOffset;

        f32 factor = ClimateNoises::factorSpline.sample(values);

        f32 initialDensity = depth * factor;
        if (initialDensity > 0.) initialDensity *= 4.;
        initialDensity -= 0.703125;
        initialDensity = std::clamp(initialDensity, -64.f, 64.f);

        f32 topGradient = yClampedGradient(y, 240, 256, 1, 0);
        f32 bottomGradient = yClampedGradient(y, -64, -40, 0, 1);

        f64 topLerp = lerp(topGradient, -0.078125, initialDensity);
        f64 bottomLerp = lerp(bottomGradient, 0.1171875, topLerp);
        return bottomLerp;
    }

    ///TODO: remove everything that is y-independent (like factor) from the y-loop to boost performance
    ///Function currently gives inaccurate results
    int estimateHeight(int x, int z) {
        //for (int i = 320; i > -64; i -= 2) {
        for (int i = 256; i > -64; i -= 8) {
            f32 initialDensity = this->sampleInitialDensity(x, i, z);
            if (initialDensity > 0.390625) return i;
        }
        return std::numeric_limits<int>::max();
    }
};

struct Noises {
    DoublePerlinNoise temperature;
    DoublePerlinNoise humidity;
    DoublePerlinNoise continentalness;
    DoublePerlinNoise erosion;
    DoublePerlinNoise weirdness;
    DoublePerlinNoise offset;
    DoublePerlinNoise surface;

    u64 seedLo, seedHi;

    explicit Noises(i64 worldSeed) {
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

    inline f32 shiftX(f32 x, f32 z) const {
        return this->offset.sample(x, 0, z) * 4.;
    }

    inline f32 shiftZ(f32 x, f32 z) const {
        return this->offset.sample(z, x, 0) * 4.;
    }

    NoisePoint sampleForBiomes(const i32 x, const i32 y, const i32 z, const bool biome_scale = false, const bool no_shift = false, const bool no_depth = false) const {
        f32 inX = biome_scale ? x : (x >> 2), inY = biome_scale ? y : (y >> 2), inZ = biome_scale ? z : (z >> 2);
        if (!no_shift) {
            inX += this->shiftX(inX, inZ);
            inZ += this->shiftZ(inX, inZ);
        }

        f32 temperatureOut = (f32)(this->temperature.sample(inX, 0, inZ));
        f32 humidityOut = (f32)(this->humidity.sample(inX, 0, inZ));
        f32 continentalnessOut = (f32)(this->continentalness.sample(inX, 0, inZ));
        f32 erosionOut = (f32)(this->erosion.sample(inX, 0, inZ));
        f32 weirdnessOut = (f32)(this->weirdness.sample(inX, 0, inZ));

        f32 depth = 0.f;
        if (!no_depth) {
            std::array<f32, 4> params = {continentalnessOut, erosionOut, static_cast<f32>(pvTransform(weirdnessOut)), weirdnessOut};
            f32 depthOffset = offsetSpline()->sample(params) - 0.50375f;
            depth = yClampedGradient(inY * 4., -64, 320, 1.5, -1.5) + depthOffset;
        }
        
        return NoisePoint(temperatureOut * 10000.f, humidityOut * 10000.f, continentalnessOut * 10000.f, erosionOut * 10000.f, depth * 10000.f, weirdnessOut * 10000.f);
    }
};

#endif