#ifndef NOISES_HPP
#define NOISES_HPP

#include <mcmapper/rng/noise.hpp>
#include <mcmapper/biome/biometree.hpp>
#include <mcmapper/terrain/spline.hpp>

#include <numeric>
#include <iostream>

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

    const std::shared_ptr<Spline> offsetSp = offsetSpline();
    const std::shared_ptr<Spline> factorSp = factorSpline();
    
    const std::shared_ptr<SearchTree> biomeSearchTree = getSearchTree();

    u64 seedLo, seedHi;

    ClimateNoises(i64 worldSeed);

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
            f32 depthOffset = ClimateNoises::offsetSp->sample(params) - 0.50375f;
            depth = yClampedGradient(inY * 4., -64, 320, 1.5, -1.5) + depthOffset;
        }
        
        return NoisePoint(temperatureOut, humidityOut, continentalnessOut, erosionOut, depth, weirdnessOut);
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

        f32 depthOffset = ClimateNoises::offsetSp->sample(values) - 0.50375f;
        f32 depth = yClampedGradient(y, -64, 320, 1.5, -1.5) + depthOffset;

        f32 factor = ClimateNoises::factorSp->sample(values);

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
    // Climate noises
    DoublePerlinNoise temperature;
    DoublePerlinNoise humidity;
    DoublePerlinNoise continentalness;
    DoublePerlinNoise erosion;
    DoublePerlinNoise weirdness;
    DoublePerlinNoise offset;

    // Surface noises
    DoublePerlinNoise surface;

    // Cave noises
    DoublePerlinNoise spaghetti3d_1;
    DoublePerlinNoise spaghetti3d_2;
    DoublePerlinNoise spaghetti3d_rarity;
    DoublePerlinNoise spaghetti3d_thickness;
    DoublePerlinNoise spaghetti_roughness;
    DoublePerlinNoise spaghetti_roughness_modulator;
    DoublePerlinNoise cave_entrance;
    DoublePerlinNoise cave_layer;
    DoublePerlinNoise cave_cheese;

    u64 seedLo, seedHi;

    explicit Noises(i64 worldSeed);

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
        
        return NoisePoint(temperatureOut, humidityOut, continentalnessOut, erosionOut, depth, weirdnessOut);
    }
};

#endif