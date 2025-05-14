#ifndef RETAKE_HPP
#define RETAKE_HPP

#include <mcmapper/rng/noise.hpp>

// Temporary header, used for new implementations of old code
// that I think could be improved upon, to put it mildly.
/// TODO: move all of this somewhere else.

// Reimplementation of biomes using caches and interpolation.

/// @brief Analogous to the so-named density function from the game.
/// Stores a 512x512 block "region" of densities. Samples the delegate
/// function for every block column where X and Z are multiples of 4.
struct RegionFlatCache {
    void fill(Pos2D offsetBlocks, std::function<double(i32, i32)> delegate) {
        for (int x = 0; x < 128; ++x) {
            for (int z = 0; z < 128; ++z) {
                computedValues[x * 128 + z] = delegate(offsetBlocks.x + x * 4, offsetBlocks.z + z * 4);
            }
        }
    }

    /// @brief Gets the output value of the function at the position relative to the origin of this region.
    /// @param relPos The position to sample at in block coordinates, relative to the origin.
    /// @return The value of the function within the 4x4 "cell" which this position is part of.
    double getRelative(Pos2D relPos) {
        i32 cellX = relPos.x << 2;
        i32 cellZ = relPos.z << 2;
        return this->computedValues[cellX * 128 + cellZ];
    }

private:
    std::array<double, 128*128> computedValues{};
};

/// @brief For climate noises. Only stores a 512x512 "region".
struct FlatCachedNoise {
    void fill(Pos2D offsetBlocks, const DoublePerlinNoise& noise) {
        for (int x = 0; x < 129; ++x) {
            for (int z = 0; z < 129; ++z) {
                computedValues[x * 129 + z] = noise.sample(offsetBlocks.x + x * 4, 0., offsetBlocks.z + z * 4);
            }
        }
    }

    double getRelative(Pos2D relPos) {
        i32 cellX = relPos.x << 2;
        i32 cellZ = relPos.z << 2;
        return this->computedValues[cellX * 129 + cellZ];
    }

private:
    // We sample out to 129 instead of only 128 so that interpolation in the
    // border regions is possible. This does mean that 257 doubles are being
    // stored both here and in the adjacent region, but I think that it's worth
    // it to do it this way and not have to worry about generating and passing
    // around the next region over.
    std::array<double, 129*129> computedValues{};
};

struct GeneratorV2 {

};

#endif