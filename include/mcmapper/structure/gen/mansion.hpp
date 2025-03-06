#ifndef MANSION_HPP
#define MANSION_HPP

#include <mcmapper/rng/rng.hpp>

template <u32 X_SIZE, u32 Y_SIZE>
struct FlagMatrix {
    explicit FlagMatrix(u8 defaultValue) : defaultValue(defaultValue) {}

    void set(u32 x, u32 y, u8 value);
    void fill(u32 fromX, u32 fromZ, u32 toX, u32 toZ, u8 value);
    void get(u32 x, u32 y);
    void update(u32 x, u32 y, u8 expected, u8 newValue);

private:
    std::array<std::array<u8, Y_SIZE>, X_SIZE> flags;
    u8 defaultValue;
};

struct MansionLayout {
    explicit MansionLayout(Random& random);

private:
    FlagMatrix<11, 11> baseLayout = FlagMatrix<11, 11>(5);
};

#endif