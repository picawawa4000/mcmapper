#ifndef RETAKE_HPP
#define RETAKE_HPP

#include <mcmapper/rng/noise.hpp>
#include <mcmapper/biome/tree/biometree.hpp>

// Temporary header, used for new implementations of old code
// that I think could be improved upon, to put it mildly.
/// TODO: move all of this somewhere else.

// Reimplementation of biomes using caches and interpolation.

/// @brief Analogous to the so-named density function from the game.
/// Stores a 512x512 block "region" of densities. Samples the delegate
/// function for every block column where X and Z are multiples of 4.
struct RegionFlatCache {
    bool hasSampledCaches = false;

    void fill(Pos2D offsetBlocks, std::function<double(i32, i32)> delegate) {
        for (int x = 0; x < 128; ++x) {
            for (int z = 0; z < 128; ++z) {
                computedValues[x * 128 + z] = delegate(offsetBlocks.x + x * 4, offsetBlocks.z + z * 4);
            }
        }

        this->hasSampledCaches = true;
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
    bool hasSampledCaches = false;

    void fill(Pos2D offsetBlocks, const DoublePerlinNoise& noise) {
        for (int x = 0; x < 129; ++x) {
            for (int z = 0; z < 129; ++z) {
                computedValues[x * 129 + z] = noise.sample(offsetBlocks.x + x * 4, 0., offsetBlocks.z + z * 4);
            }
        }

        this->hasSampledCaches = true;
    }

    /// @brief Gets a value at a position relative to the origin.
    /// @param relPos The position in blocks, relative to the origin of this `FlatCachedNoise`. Must not have negative coordinates.
    /// @return The value at the cell containing the position.
    double getRelative(Pos2D relPos) {
        u32 cellX = relPos.x << 2;
        u32 cellZ = relPos.z << 2;
        return this->computedValues[cellX * 129 + cellZ];
    }

    // See note on `getRelative`.
    double getInterpolated(Pos2D relPos) {
        u32 cellX = relPos.x << 2;
        u32 cellZ = relPos.z << 2;
        u32 localX = relPos.x % 4;
        u32 localZ = relPos.z % 4;
        if (localX == 0 && localZ == 0) return this->computedValues[cellX * 129 + cellZ];
        return lerp2(localX, localZ, this->computedValues[cellX * 129 + cellZ], this->computedValues[cellX * 129 + cellZ + 129], this->computedValues[cellX * 129 + cellZ + 1], this->computedValues[cellX * 129 + cellZ + 130]);
    }

private:
    // We sample out to 129 instead of only 128 so that interpolation in the
    // border regions is possible. This does mean that 257 doubles are being
    // stored both here and in the adjacent region, but I think that it's worth
    // it to do it this way and not have to worry about generating and passing
    // around the next region over.
    std::array<double, 129*129> computedValues{};
};

struct BiomeGenerator {
    BiomeGenerator() = default;
    BiomeGenerator(std::shared_ptr<FlatCachedNoise> temperature, std::shared_ptr<FlatCachedNoise> humidity, std::shared_ptr<FlatCachedNoise> continentalness, std::shared_ptr<FlatCachedNoise> erosion, std::shared_ptr<FlatCachedNoise> weirdness)
        : temperature(temperature), humidity(humidity), continentalness(continentalness), erosion(erosion), weirdness(weirdness) {}
    
    void setSeed(u64 seed) {
        throw std::runtime_error("unimplemented function setSeed!");
    }
    
    /// @brief Modifies the internal search tree without resampling.
    /// @param newTree A shared pointer to the new search tree.
    void setTree(std::shared_ptr<SearchTree> newTree) {
        this->tree = newTree;
    }

    Biome getRelative(i32 x, i32 y, i32 z) {
        throw std::runtime_error("unimplemented function getRelative!");
    }

private:
    std::shared_ptr<FlatCachedNoise> temperature, humidity, continentalness, erosion, weirdness;
    std::shared_ptr<SearchTree> tree;
};

#endif