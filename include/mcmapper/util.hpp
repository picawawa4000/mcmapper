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

inline f64 lerp2(f64 dx, f64 dy, f64 v00, f64 v10, f64 v01, f64 v11) {
    return lerp(dy, lerp(dx, v00, v10), lerp(dx, v01, v11));
}

inline f64 lerp3(f64 dx, f64 dy, f64 dz, f64 v000, f64 v100, f64 v010, f64 v110, f64 v001, f64 v101, f64 v011, f64 v111) {
    v000 = lerp2(dx, dy, v000, v100, v010, v110);
    v001 = lerp2(dx, dy, v001, v101, v011, v111);
    return lerp(dz, v000, v001);
}

inline f64 perlinFade(f64 value) {
    return ((value * 6. - 15.) * value + 10.) * value * value * value;
}

inline f64 clamp(f64 value, f64 lower, f64 upper) {
    if (value < lower) return lower;
    if (value > upper) return upper;
    return value;
}

struct Pos2D {
    i32 x;
    i32 z;

    Pos2D(i32 x, i32 z) : x(x), z(z) {}

    std::string toString() {
        return "(" + std::to_string(this->x) + ", " + std::to_string(this->z) + ")";
    }

    Pos2D chunkToBlock() {
        return Pos2D(this->x << 4, this->z << 4);
    }

    bool operator<(Pos2D other) const {
        return this->x * this->x + this->z * this->z < other.x * other.x + other.z * other.z;
    }

    operator std::string() {
        return this->toString();
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

//helper function for writing Pos2Ds to std::cout (and other streams)
inline std::ostream& operator<<(std::ostream& stream, Pos2D pos) {
    stream << (std::string)pos;
    return stream;
}

//helper function for writing Pos3Ds to std::cout (and other streams)
inline std::ostream& operator<<(std::ostream& stream, Pos3D pos) {
    stream << (std::string)pos;
    return stream;
}

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

inline f32 getLerpProgress(f32 value, f32 start, f32 end) {
    return (value - start) / (end - start);
}

inline f32 clampedLerp(f32 start, f32 end, f32 delta) {
    if (delta < 0.0) return start;
    if (delta > 1.0) return end;
    return lerp(delta, start, end);
}

inline f32 yClampedGradient(f32 y, i32 fromY, i32 toY, f32 fromValue, f32 toValue) {
    return clampedLerp(fromValue, toValue, getLerpProgress(y, fromY, toY));
}

inline i64 hashCode(i32 x, i32 y, i32 z) {
    i64 l = (i64)(x * 3129871) ^ (i64)z * 116129781L ^ (i64)y;
    l = l * l * 42317861L + l * 11L;
    return l >> 16;
}

enum Direction {
    NORTH,
    SOUTH,
    EAST,
    WEST,
};

u8 getOffsetX(Direction direction) {
    throw std::runtime_error("Unimplemented function getOffsetX!");
}

#endif