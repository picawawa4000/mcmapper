#ifndef MANSION_HPP
#define MANSION_HPP

#include <mcmapper/rng/rng.hpp>
#include <mcmapper/structure/gen/spiece.hpp>

/// TODO: de-template-ify this because X_SIZE and Y_SIZE are always 11
template <u32 X_SIZE, u32 Y_SIZE>
struct FlagMatrix {
    explicit FlagMatrix(u32 defaultValue) : defaultValue(defaultValue) {}

    void set(u32 x, u32 y, u32 value);
    void fill(u32 fromX, u32 fromZ, u32 toX, u32 toZ, u32 value);
    // This is really ugly, but it's a stopgap solution for now.
    u32 get(u32 x, u32 y) {
        if (x >= 0 && x < X_SIZE && y >= 0 && y < Y_SIZE) return this->flags[x][y];
            return this->defaultValue;
    }
    void update(u32 x, u32 y, u32 expected, u32 newValue);

private:
    std::array<std::array<u32, Y_SIZE>, X_SIZE> flags{};
    u32 defaultValue;
};

typedef FlagMatrix<11, 11> BaseMansionFlags;

struct MansionLayout {
    BaseMansionFlags firstFloor = BaseMansionFlags(5 /*OUTSIDE*/);
    BaseMansionFlags secondFloor = BaseMansionFlags(5 /*OUTSIDE*/);
    BaseMansionFlags thirdFloor = BaseMansionFlags(5 /*OUTSIDE*/);

    // `CheckedRandom& random` is the correct parameter, but too lazy to change it for now
    explicit MansionLayout(Random& random);

private:
    BaseMansionFlags baseLayout = BaseMansionFlags(5 /*OUTSIDE*/);
    BaseMansionFlags thirdFloorLayout = BaseMansionFlags(5 /*OUTSIDE*/);

    std::vector<StructurePiece> pieces;
};

#endif