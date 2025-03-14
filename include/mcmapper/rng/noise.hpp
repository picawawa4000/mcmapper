#ifndef NOISE_HPP
#define NOISE_HPP

#include <mcmapper/rng/rng.hpp>
#include <cmath>
#include <vector>
#include <memory>

struct PerlinNoise {
    f64 originX, originY, originZ;
    u8 permutation[256];

    PerlinNoise(Random& rng);
    f64 sample(f64 x, f64 y, f64 z, f64 yScale = 0, f64 yMax = 0) const;

private:
    u8 map(i32 input) const {return this->permutation[input & 0xFF];}
    f64 sample(i32 sectionX, i32 sectionY, i32 sectionZ, f64 localX, f64 localY, f64 localZ, f64 fadeLocalY) const;
};

struct OctavePerlinNoise {
    std::vector<std::shared_ptr<PerlinNoise>> octaves;
    std::vector<f64> amplitudes;
    f64 lacunarity;
    f64 persistance;

    OctavePerlinNoise(XoroshiroRandom& rng, std::vector<f64> amplitudes, i32 firstOctave);
    OctavePerlinNoise(CheckedRandom& rng, std::vector<f64> amplitudes, i32 firstOctave);
    OctavePerlinNoise() = default;

    f64 sample(f64 x, f64 y, f64 z) const;
};

struct DoublePerlinNoise {
    f64 amplitude;
    OctavePerlinNoise first, second;

    DoublePerlinNoise(CheckedRandom& rng, std::vector<f64> amplitudes, i32 firstOctave) : first(rng, amplitudes, firstOctave), second(rng, amplitudes, firstOctave) {this->construct(amplitudes);}
    DoublePerlinNoise(XoroshiroRandom& rng, std::vector<f64> amplitudes, i32 firstOctave) : first(rng, amplitudes, firstOctave), second(rng, amplitudes, firstOctave) {this->construct(amplitudes);}
    DoublePerlinNoise() = default;

    f64 sample(f64 x, f64 y, f64 z) const;

private:
    void construct(std::vector<f64> amplitudes);
};

#endif