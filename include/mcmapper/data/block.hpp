#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <unordered_map>
#include <string>

// No order determined as of now.
enum BlockType {
    AIR,
    STONE,
    COBBLESTONE,
    MOSSY_COBBLESTONE,
    OAK_PLANKS,
    SPRUCE_PLANKS,
    BIRCH_PLANKS,
    JUNGLE_PLANKS,
    ACACIA_PLANKS,
    DARK_OAK_PLANKS,
};

// The map is more efficient than it looks.
// As long as blocks aren't being passed around everywhere, I think that this will work.
// (Pointers are great.)
struct Block {
    BlockType type;
    std::unordered_map<std::string, std::string> properties;

    // Modifies the property of this block and returns a new instance.
    Block with(std::string name, std::string value) {
        Block block(*this);
        block.properties[name] = value;
        return block;
    }

    // Modifies this block in place and returns the this pointer.
    Block * withInPlace(std::string name, std::string value) {
        this->properties[name] = value;
        return this;
    }
};

#endif