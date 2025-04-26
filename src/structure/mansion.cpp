#include <mcmapper/structure/gen/mansion.hpp>

enum {
    UNSET = 0,
    CORRIDOR = 1,
    ROOM = 2,
    STAIRCASE = 3,
    UNUSED = 4,
    OUTSIDE = 5
};

namespace flags {
    const u32
        SMALL_ROOM = 0x10000,
        MEDIUM_ROOM = 0x20000,
        BIG_ROOM = 0x40000,
        ORIGIN_CELL = 0x100000,
        ENTRANCE_CELL = 0x200000,
        STAIRCASE_CELL = 0x400000,
        CARPET_CELL = 0x800000;
}

template <u32 X_SIZE, u32 Y_SIZE> void FlagMatrix<X_SIZE, Y_SIZE>::set(u32 x, u32 y, u32 value) {
    if (x >= 0 && x < X_SIZE && y >= 0 && y < Y_SIZE)
        this->flags[x][y] = value;
}

template <u32 X_SIZE, u32 Y_SIZE> void FlagMatrix<X_SIZE, Y_SIZE>::fill(u32 fromX, u32 fromY, u32 toX, u32 toY, u32 value) {
    fromX = std::max(0U, fromX);
    fromY = std::max(0U, fromY);
    toX = std::min(toX, X_SIZE - 1);
    toY = std::min(toY, Y_SIZE - 1);
    
    if (toX < fromX || toY < fromY) throw std::runtime_error("FlagMatrix::fill: Tried to fill negative area!");

    for (int i = fromX; i <= toX; ++i) for (int j = fromY; j <= toY; ++j)
        this->flags[i][j] = value;
}

template <u32 X_SIZE, u32 Y_SIZE> void FlagMatrix<X_SIZE, Y_SIZE>::update(u32 x, u32 y, u32 expected, u32 newValue) {
    if (this->get(x, y) == expected) this->set(x, y, newValue);
}

static inline Direction getRandomDirection(Random& rng) {
    i32 ordinal = rng.next_i32(4);
    return static_cast<Direction>(ordinal);
}

static inline bool opposites(Direction first, Direction second) {
    // I could do a comparison by checking whether the directions are
    // two apart (i.e. first + 2 == second || second + 2 == first),
    // but that requires the implementation of directions to be like
    // that (although I guess it's guaranteed by `getRandomDirection`).
    if (first == DIR_NORTH && second == DIR_SOUTH) return true;
    if (first == DIR_SOUTH && second == DIR_NORTH) return true;
    if (first == DIR_EAST && second == DIR_WEST) return true;
    if (first == DIR_WEST && second == DIR_EAST) return true;
    return false;
}

static void layoutCorridor(BaseMansionFlags& layout, Random& rng, u32 x, u32 y, Direction direction, u32 length) {
    if (length <= 0) return;

    layout.set(x, y, CORRIDOR);
    layout.update(x + getOffsetX(direction), y + getOffsetZ(direction), UNSET, CORRIDOR);
    for (int i = 0; i < 8; ++i) {
        Direction randomDirection = getRandomDirection(rng);
        if (
            opposites(direction, randomDirection) ||
            randomDirection == DIR_EAST && rng.next_bool()
        ) continue;
        i32 m = x + getOffsetX(direction);
        i32 n = y + getOffsetZ(direction);
        if (
            layout.get(m + getOffsetX(randomDirection), n + getOffsetZ(randomDirection)) != 0 ||
            layout.get(m + getOffsetX(randomDirection) * 2, n + getOffsetZ(randomDirection) * 2) != 0
        ) continue;
        layoutCorridor(layout, rng, x + getOffsetX(direction) + getOffsetX(randomDirection), y + getOffsetZ(direction) + getOffsetZ(randomDirection), randomDirection, length - 1);
        break;
    }
    Direction cw = clockwise(direction);
    Direction ccw = counterclockwise(direction);

    layout.update(x + getOffsetX(cw), y + getOffsetZ(cw), UNSET, ROOM);
    layout.update(x + getOffsetX(ccw), y + getOffsetZ(ccw), UNSET, ROOM);
    layout.update(x + getOffsetX(direction) + getOffsetX(cw), y + getOffsetZ(direction) + getOffsetZ(cw), UNSET, ROOM);
    layout.update(x + getOffsetX(direction) + getOffsetX(ccw), y + getOffsetZ(direction) + getOffsetZ(ccw), UNSET, ROOM);
    layout.update(x + getOffsetX(direction) * 2, y + getOffsetZ(direction) * 2, UNSET, ROOM);
    layout.update(x + getOffsetX(cw) * 2, y + getOffsetZ(cw) * 2, UNSET, ROOM);
    layout.update(x + getOffsetX(ccw) * 2, y + getOffsetZ(ccw) * 2, UNSET, ROOM);
}

static inline bool isInsideMansion(BaseMansionFlags& layout, int x, int y) {
    u8 val = layout.get(x, y);
    return val == CORRIDOR || val == ROOM || val == STAIRCASE || val == UNUSED;
}

static bool adjustLayoutWithRooms(BaseMansionFlags& layout) {
    bool adjusted = false;
    for (int i = 0; i < 11; ++i) {
        for (int j = 0; j < 11; ++j) {
            if (layout.get(j, i) != UNSET) continue;
            int k = 0;
            // implicit bool-to-int cast (game uses an explicit one because Java)
            // false -> 0, true -> 1
            k += isInsideMansion(layout, j + 1, i);
            k += isInsideMansion(layout, j - 1, i);
            k += isInsideMansion(layout, j, i + 1);
            k += isInsideMansion(layout, j, i - 1);
            if (k >= 3) {
                layout.set(j, i, ROOM);
                adjusted = true;
                continue;
            }
            if (k != 2) continue;
            k = 0;
            k += isInsideMansion(layout, j + 1, i + 1);
            k += isInsideMansion(layout, j - 1, i + 1);
            k += isInsideMansion(layout, j + 1, i - 1);
            k += isInsideMansion(layout, j - 1, i - 1);
            if (k > 1) continue;
            layout.set(j, i, ROOM);
            adjusted = true;
        }
    }
    return adjusted;
}

static void shuffleInPlace(std::vector<std::pair<u32, u32>>& vec, Random& rng) {
    // Knuth shuffle
    for (int i = vec.size(); i > 1; --i) {
        int j = rng.next_i32(i);
        std::pair<u32, u32> temp = vec[i - 1];
        vec[i - 1] = vec[j];
        vec[j] = temp;
    }
}

static bool anyMatchAround(BaseMansionFlags& flags, u32 x, u32 y, u32 value) {
    if (flags.get(x + 1, y) == value || flags.get(x, y + 1) == value || flags.get(x - 1, y) == value || flags.get(x, y - 1) == value) return true;
    return false;
}

static void updateRoomFlags(BaseMansionFlags& baseLayout, BaseMansionFlags& floor, Random& rng) {
    std::vector<std::pair<u32, u32>> rooms;
    for (int i = 0; i < 11; ++i) {
        for (int j = 0; j < 11; ++j) {
            if (baseLayout.get(j, i) != ROOM) continue;
            rooms.emplace_back(j, i);
        }
    }
    shuffleInPlace(rooms, rng);

    u32 i = 10;
    for (std::pair<u32, u32>& room : rooms) {
        u32 x = room.first;
        u32 y = room.second;
        if (floor.get(x, y) != 0) continue;

        i32 minX = x, maxX = x, minY = y, maxY = y, sizeFlag = flags::SMALL_ROOM;
        // attempt to place big room
        if (
            // none of the adjacent cells are set already?
            floor.get(x + 1, y) == 0 && floor.get(x, y + 1) == 0 && floor.get(x + 1, y + 1) == 0 &&
            // every adjacent cell is supposed to be a room?
            baseLayout.get(x + 1, y) == ROOM && baseLayout.get(x, y + 1) == ROOM && baseLayout.get(x + 1, y + 1) == ROOM
        ) {
            ++maxX; ++maxY; sizeFlag = flags::BIG_ROOM;
        } else if (
            // try the other directions for big room
            floor.get(x - 1, y) == 0 && floor.get(x, y + 1) == 0 && floor.get(x - 1, y + 1) == 0 &&
            baseLayout.get(x - 1, y) == ROOM && baseLayout.get(x, y + 1) == ROOM && baseLayout.get(x - 1, y + 1) == ROOM
        ) {
            --minX; ++maxY; sizeFlag = flags::BIG_ROOM;
        } else if (
            // try the other directions for big room
            floor.get(x - 1, y) == 0 && floor.get(x, y - 1) == 0 && floor.get(x - 1, y - 1) == 0 &&
            baseLayout.get(x - 1, y) == ROOM && baseLayout.get(x, y - 1) == ROOM && baseLayout.get(x - 1, y - 1) == ROOM
        ) {
            --minX; --minY; sizeFlag = flags::BIG_ROOM;
        // try for medium room
        } else if (floor.get(x + 1, y) == 0 && baseLayout.get(x + 1, y) == ROOM) {
            ++maxX; sizeFlag = flags::MEDIUM_ROOM;
        } else if (floor.get(x, y + 1) == 0 && baseLayout.get(x, y + 1) == ROOM) {
            ++maxY; sizeFlag = flags::MEDIUM_ROOM;
        } else if (floor.get(x - 1, y) == 0 && baseLayout.get(x - 1, y) == ROOM) {
            --minX; sizeFlag = flags::MEDIUM_ROOM;
        } else if (floor.get(x, y - 1) == 0 && baseLayout.get(x, y - 1) == ROOM) {
            --minY; sizeFlag = flags::MEDIUM_ROOM;
        }

        u32 r = rng.next_bool() ? minX : maxX;
        u32 s = rng.next_bool() ? minY : maxY;
        u32 t = flags::ENTRANCE_CELL;
        if (!anyMatchAround(baseLayout, r, s, CORRIDOR)) {
            r = r == minX ? maxX : minX;
            s = s == minY ? maxY : minY;
            if (!anyMatchAround(baseLayout, r, s, CORRIDOR)) {
                s = s == minY ? maxY : minY;
                if (!anyMatchAround(baseLayout, r, s, CORRIDOR)) {
                    r = r == minX ? maxX : minX;
                    s = s == minY ? maxY : minY;
                    if (!anyMatchAround(baseLayout, r, s, CORRIDOR)) {
                        t = 0;
                        r = minX;
                        s = minY;
                    }
                }
            }
        }
        
        for (int u = minY; u <= maxY; ++u) {
            for (int v = minX; v <= maxX; ++v) {
                if (v == r && u == s)
                    floor.set(v, u, flags::ORIGIN_CELL | t | sizeFlag | i);
                else
                    floor.set(v, u, sizeFlag | i);
            }
        }

        ++i;
    }
}

static inline Direction findConnectedRoomDirection(BaseMansionFlags& floor, int i, int j, u32 roomId) {
    for (int d = 0; d < 4; ++d) {
        Direction direction = static_cast<Direction>(d);
        u32 trialRoomId = floor.get(i + getOffsetX(direction), j + getOffsetZ(direction)) & 0xFFFF;
        if (trialRoomId == roomId) return direction;
    }
    throw std::runtime_error("If this point is reached, a function is called on a null variable in the Java game code, causing the game to crash.");
}

static void layoutThirdFloor(BaseMansionFlags& thirdFloorLayout, BaseMansionFlags& thirdFloor, BaseMansionFlags& secondFloor, BaseMansionFlags& baseLayout, Random& rng) {
    std::vector<std::pair<u32, u32>> candidates;
    for (int i = 0; i < 11; ++i) {
        for (int j = 0; j < 11; ++j) {
            u32 val = secondFloor.get(j, i);
            if ((val & 0xf0000) != flags::MEDIUM_ROOM || (val & flags::ENTRANCE_CELL) != flags::ENTRANCE_CELL) continue;
            candidates.emplace_back(j, i);
        }
    }
    if (candidates.empty()) {
        thirdFloorLayout.fill(0, 0, 11, 11, OUTSIDE);
        return;
    }

    std::pair<u32, u32> stairPos = candidates[rng.next_i32(candidates.size())];
    u32 i = stairPos.first;
    u32 j = stairPos.second;
    u32 k = secondFloor.get(i, j);
    secondFloor.set(i, j, k | flags::STAIRCASE_CELL);
    Direction direction = findConnectedRoomDirection(secondFloor, i, j, k & 0xFFFF);
    u32 l = i + getOffsetX(direction);
    u32 m = j + getOffsetZ(direction);
    for (int n = 0; n < 11; ++n) {
        for (int o = 0; o < 11; ++o) {
            if (!(baseLayout.get(n, o) == ROOM || baseLayout.get(n, o) == CORRIDOR || baseLayout.get(n, o) == STAIRCASE || baseLayout.get(n, o) == UNUSED)) {
                thirdFloorLayout.set(n, o, OUTSIDE);
                continue;
            }
            if (n == i && o == j) {
                thirdFloorLayout.set(n, o, STAIRCASE);
            }
            if (n != l || o != m) continue;
            thirdFloorLayout.set(n, o, STAIRCASE);
            thirdFloor.set(n, o, flags::CARPET_CELL);
        }
    }

    std::vector<Direction> directions;
    // Iterate through array {DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST}.
    for (int dir = 2; dir < 6; ++dir) {
        direction = static_cast<Direction>(dir & 0b11);
        if (thirdFloorLayout.get(l + getOffsetX(direction), m + getOffsetZ(direction)) == UNSET)
            directions.push_back(direction);
    }
    if (directions.empty()) {
        thirdFloorLayout.fill(0, 0, 11, 11, OUTSIDE);
        secondFloor.set(i, j, k);
        return;
    }
    direction = directions[rng.next_i32(directions.size())];
    layoutCorridor(thirdFloorLayout, rng, l + getOffsetX(direction), m + getOffsetZ(direction), direction, 4);
    while (adjustLayoutWithRooms(thirdFloorLayout));
}

/// TODO: make all of these subroutines (i.e. updateRoomFlags, layoutCorridor)
/// into member functions of MansionLayout because this is ugly and probably slow
MansionLayout::MansionLayout(Random& rng) {
    this->baseLayout.fill(7, 4, 8, 5, STAIRCASE);
    this->baseLayout.fill(6, 4, 6, 5, ROOM);
    this->baseLayout.fill(9, 2, 10, 7, OUTSIDE);
    this->baseLayout.fill(8, 2, 8, 3, CORRIDOR);
    this->baseLayout.fill(8, 6, 8, 7, CORRIDOR);
    this->baseLayout.set(6, 3, CORRIDOR);
    this->baseLayout.set(6, 6, CORRIDOR);
    this->baseLayout.fill(0, 0, 11, 1, OUTSIDE);
    this->baseLayout.fill(0, 9, 11, 11, OUTSIDE);

    layoutCorridor(this->baseLayout, rng, 7, 2, DIR_WEST, 6);
    layoutCorridor(this->baseLayout, rng, 7, 7, DIR_WEST, 6);
    layoutCorridor(this->baseLayout, rng, 5, 3, DIR_WEST, 3);
    layoutCorridor(this->baseLayout, rng, 5, 6, DIR_WEST, 3);

    while (adjustLayoutWithRooms(this->baseLayout));

    updateRoomFlags(this->baseLayout, this->firstFloor, rng);
    updateRoomFlags(this->baseLayout, this->secondFloor, rng);

    this->firstFloor.fill(8, 4, 8, 5, flags::CARPET_CELL);
    this->secondFloor.fill(8, 4, 8, 5, flags::CARPET_CELL);

    layoutThirdFloor(this->thirdFloorLayout, this->thirdFloor, this->secondFloor, this->baseLayout, rng);
    updateRoomFlags(this->thirdFloorLayout, this->thirdFloor, rng);
}
