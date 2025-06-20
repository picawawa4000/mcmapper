#ifndef UTIL_HPP
#define UTIL_HPP

#include <cinttypes>
#include <string>
#include <algorithm>
#include <vector>
#include <ostream>

typedef int8_t      i8;
typedef uint8_t     u8;
typedef int16_t     i16;
typedef uint16_t    u16;
typedef int32_t     i32;
typedef uint32_t    u32;
typedef int64_t     i64;
typedef uint64_t    u64;
typedef float       f32;
typedef double      f64;

inline u64 rol(u64 input, u32 shift) {
    if ((shift &= 63) == 0) return input;
    return (input << shift) | (input >> (64 - shift));
}

inline f64 lerp(f64 part, f64 from, f64 to) {
    return from + part * (to - from);
}

// vxy (for example, v10: x = 1, y = 0)
inline f64 lerp2(f64 dx, f64 dy, f64 v00, f64 v10, f64 v01, f64 v11) {
    return lerp(dy, lerp(dx, v00, v10), lerp(dx, v01, v11));
}

// see comment on lerp2
inline f64 lerp3(f64 dx, f64 dy, f64 dz, f64 v000, f64 v100, f64 v010, f64 v110, f64 v001, f64 v101, f64 v011, f64 v111) {
    v000 = lerp2(dx, dy, v000, v100, v010, v110);
    v001 = lerp2(dx, dy, v001, v101, v011, v111);
    return lerp(dz, v000, v001);
}

inline f64 perlinFade(f64 value) {
    return ((value * 6. - 15.) * value + 10.) * value * value * value;
}

struct Pos2D {
    i32 x;
    i32 z;

    Pos2D(i32 x, i32 z) : x(x), z(z) {}

    std::string toString() const {
        return "(" + std::to_string(this->x) + ", " + std::to_string(this->z) + ")";
    }

    Pos2D chunkToBlock() const {
        return Pos2D(this->x << 4, this->z << 4);
    }

    bool operator<(Pos2D other) const {
        return this->x * this->x + this->z * this->z < other.x * other.x + other.z * other.z;
    }

    operator std::string() const {
        return this->toString();
    }

    bool operator ==(const Pos2D& other) const {
        return this->x == other.x && this->z == other.z;
    }
};

// Pos3D isn't as easily hashable because there are more than 64 bits.
template<> struct std::hash<Pos2D> {
    // Relies on 64-bit std::size_t.
    std::size_t operator()(const Pos2D& input) const noexcept {
        return ((u64)input.x) << 32 | (u64)input.z;
    }
};

struct Pos3D {
    i32 x;
    i32 y;
    i32 z;

    Pos3D(i32 x, i32 y, i32 z) : x(x), y(y), z(z) {}

    std::string toString() {
        return "(" + std::to_string(this->x) + ", " + std::to_string(this->y) + ", " + std::to_string(this->z) + ")";
    }

    operator std::string() {
        return this->toString();
    }
};

template <typename T> struct Tag {
    std::vector<T> data;

    Tag() : data() {}
    Tag(std::vector<T> initial) : data(initial) {}

    Tag add(T t) {
        this->data.push_back(t);
        return *this;
    }

    Tag add(std::vector<T> t) {
        this->data.insert(this->data.end(), t.begin(), t.end());
        return *this;
    }

    Tag add(Tag<T> t) {
        this->data.insert(this->data.end(), t.data.begin(), t.data.end());
        return *this;
    }

    Tag remove(T t) {
        auto last_index = std::remove(this->data.begin(), this->data.end(), t);
        this->data.erase(last_index, this->data.end());
        return *this;
    }

    bool contains(T t) const {
        return std::find(this->data.begin(), this->data.end(), t) != this->data.end();
    }
};

inline double pvTransform(double weirdness) {
    return -3.f * (std::abs(std::abs(weirdness) - 0.6666667) - 0.33333334);
}

inline f64 getLerpProgress(f64 value, f64 start, f64 end) {
    return (value - start) / (end - start);
}

inline f64 clampedLerp(f64 start, f64 end, f64 delta) {
    if (delta < 0.0) return start;
    if (delta > 1.0) return end;
    return lerp(delta, start, end);
}

inline f64 yClampedGradient(f64 y, i32 fromY, i32 toY, f64 fromValue, f64 toValue) {
    return clampedLerp(fromValue, toValue, getLerpProgress(y, fromY, toY));
}

inline i64 hashCode(i32 x, i32 y, i32 z) {
    i64 l = (i64)(x * 3129871) ^ (i64)z * 116129781L ^ (i64)y;
    l = l * l * 42317861L + l * 11L;
    return l >> 16;
}

// Ordered this way so that `static_cast<Direction>(i)` is equivalent
// to `Direction.fromHorizontalQuarterTurns(i)`.
enum Direction {
    DIR_SOUTH = 0,
    DIR_WEST = 1,
    DIR_NORTH = 2,
    DIR_EAST = 3,
};

inline i8 getOffsetX(Direction direction) {
    if (direction == DIR_WEST) return -1;
    if (direction == DIR_EAST) return 1;
    return 0;
}

inline i8 getOffsetZ(Direction direction) {
    if (direction == DIR_NORTH) return -1;
    if (direction == DIR_SOUTH) return 1;
    return 0;
}

inline Direction clockwise(Direction direction) {
    return static_cast<Direction>(static_cast<int>(direction) + 1 & 0b11);
}

inline Direction counterclockwise(Direction direction) {
    return static_cast<Direction>(static_cast<int>(direction) - 1 & 0b11);
}

enum Dimension {
    DIM_OVERWORLD,
    DIM_NETHER,
    DIM_END
};

static inline double squeeze(double density) {
    double c = std::clamp(density, -1., 1.);
    return c / 2. - c * c * c / 24.;
}

#endif