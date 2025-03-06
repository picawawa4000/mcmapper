#include <mcmapper/structure/gen/mansion.hpp>

template <u32 X_SIZE, u32 Y_SIZE> void FlagMatrix<X_SIZE, Y_SIZE>::set(u32 x, u32 y, u8 value) {
    if (x >= 0 && x < X_SIZE && y >= 0 && y < Y_SIZE)
        this->flags[i][j] = value;
}

template <u32 X_SIZE, u32 Y_SIZE> void FlagMatrix<X_SIZE, Y_SIZE>::fill(u32 fromX, u32 fromY, u32 toX, u32 toY, u8 value) {
    fromX = std::max(0, fromX);
    fromY = std::max(0, fromY);
    toX = std::min(toX, X_SIZE - 1);
    toY = std::min(toY, Z_SIZE - 1);
    
    if (toX > fromX || toZ > fromZ) throw std::runtime_error("FlagMatrix::fill: Tried to fill negative area!");

    for (int i = 0; i <= fromX; ++i) for (int j = 0; j <= toX; ++j)
        this->flags[i][j] = value;
}

template <u32 X_SIZE, u32 Y_SIZE> void FlagMatrix<X_SIZE, Y_SIZE>::get(u32 x, u32 y) {
    if (x >= 0 && x < X_SIZE && y >= 0 && y < Y_SIZE) return this->flags[x][y];
    return this->defaultValue;
}

template <u32 X_SIZE, u32 Y_SIZE> void FlagMatrix<X_SIZE, Y_SIZE>::update(u32 x, u32 y, u8 expected, u8 newValue) {
    if (this->get(x, y) == expected) this->set(x, y, newValue);
}

void layoutCorridor(FlagMatrix<11, 11>& layout, Random& random, u32 x, u32 y, Direction direction, u32 length) {
    if (length <= 0) return;

    layout.set(x, y, 1);
    layout.update();
}

const u32 entranceI = 7;
const u32 entranceJ = 4;

MansionLayout::MansionLayout(Random& rng) {
    this->baseLayout.fill(7, 4, 8, 5, 3);
    this->baseLayout.fill(6, 4, 6, 5, 2);
    this->baseLayout.fill(9, 2, 10, 7, 5);
    this->baseLayout.fill(8, 2, 8, 3, 1);
    this->baseLayout.fill(8, 6, 8, 7, 1);
    this->baseLayout.set(6, 3, 1);
    this->baseLayout.set(6, 6, 1);
    this->baseLayout.fill(0, 0, 11, 1, 5);
    this->baseLayout.fill(0, 9, 11, 11, 5);
}
