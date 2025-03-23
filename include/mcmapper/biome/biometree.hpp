#ifndef BIOMETREE_HPP
#define BIOMETREE_HPP

#include <mcmapper/biome/biomes.hpp>
#include <vector>
#include <optional>
#include <utility>
#include <cmath>
#include <limits>
#include <functional>

#include <fstream>
#include <iostream>

struct NoisePoint {
    i64 temperature;
    i64 humidity;
    i64 continentalness;
    i64 erosion;
    i64 depth;
    i64 weirdness;

    explicit NoisePoint(i64 temperature, i64 humidity, i64 continentalness, i64 erosion, i64 depth, i64 weirdness) : temperature(temperature), humidity(humidity), continentalness(continentalness), erosion(erosion), depth(depth), weirdness(weirdness) {}
    explicit NoisePoint(f32 temperature, f32 humidity, f32 continentalness, f32 erosion, f32 depth, f32 weirdness) : temperature(temperature * 10000), humidity(humidity * 10000), continentalness(continentalness * 10000), erosion(erosion * 10000), depth(depth * 10000), weirdness(weirdness * 10000) {}

    std::vector<i64> toList() {
        return {this->temperature, this->humidity, this->continentalness, this->erosion, this->depth, this->weirdness, 0};
    }
};

struct ParameterRange {
    i64 min;
    i64 max;

    explicit constexpr ParameterRange(i64 min, i64 max) : min(min), max(max) {}
    explicit constexpr ParameterRange(f32 min, f32 max) : min((i64)(min * 10000.f)), max((i64)(max * 10000.f)) {}
    ParameterRange() : min(0), max(0) {}

    ParameterRange combine(std::optional<ParameterRange> other) const {
        return other.has_value() ? ParameterRange(std::min(this->min, other.value().min), std::max(this->max, other.value().max)) : *this;
    }

    i64 getDistance(i64 point) const {
        if (point - this->max > 0) return point - this->max;
        return std::max(this->min - point, 0LL);
    }

    operator std::string() const {
        return "[" + std::to_string(this->min) + ", " + std::to_string(this->max) + "]";
    }

    bool operator==(const ParameterRange& other) const {
        return other.min == this->min && other.max == this->max;
    }
};

struct NoiseHypercube {
    ParameterRange temperature;
    ParameterRange humidity;
    ParameterRange continentalness;
    ParameterRange erosion;
    ParameterRange depth;
    ParameterRange weirdness;
    i64 offset;

    NoiseHypercube(ParameterRange temperature, ParameterRange humidity, ParameterRange continentalness, ParameterRange erosion, ParameterRange depth, ParameterRange weirdness, i64 offset) : temperature(temperature), humidity(humidity), continentalness(continentalness), erosion(erosion), depth(depth), weirdness(weirdness), offset(offset) {}

    std::vector<ParameterRange> toList() {
        return {this->temperature, this->humidity, this->continentalness, this->erosion, this->depth, this->weirdness, ParameterRange(this->offset, this->offset)};
    }
};

struct TreeNode {
    std::vector<ParameterRange> parameters;
    std::vector<std::shared_ptr<TreeNode>> children;
    // 0 if this node is a branch
    Biome value;

    TreeNode(std::vector<ParameterRange> parameters, Biome value, std::vector<std::shared_ptr<TreeNode>> children) : parameters(parameters), children(children), value(value) {}

    TreeNode getResultingNode(std::vector<i64> point, const TreeNode& alternative);
};

/// TODO: Serialise this structure into binary and then move the generator to a separate directory (or maybe even repository)
struct SearchTree {
    std::shared_ptr<TreeNode> root;

    SearchTree(std::vector<std::pair<NoiseHypercube, Biome>> entries);

    ~SearchTree() {
        if (this->root.use_count() == 1) this->root.reset();
    }

    Biome get(NoisePoint point) const;
};

// might replace this function with a simple variable
const std::shared_ptr<SearchTree> getSearchTree();

#endif