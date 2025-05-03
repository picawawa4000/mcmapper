#ifndef SPIECE_HPP
#define SPIECE_HPP

#include <mcmapper/util.hpp>
#include <mcmapper/data/block.hpp>

#include <functional>

struct StructurePiece {
    // Use blocks[y*xSize*zSize + z*xSize + x] to index
    int xSize, zSize;
    std::vector<Block> palette;
    std::vector<u8> data;

    StructurePiece() = default;
    // U8s may seem excessive, but structures are pretty small
    StructurePiece(const std::vector<Block>& palette, const std::vector<u8>& data) {
        this->palette = palette;
        this->data = data;
    }
};

struct GeneratedPiece {
    int originX, originY, originZ;
    // DIR_SOUTH is facing forwards
    // the rest are clockwise rotations
    Direction rotation;
    StructurePiece * data;
};

struct StructurePiecesList {
    // Make sure that pieces exists and does not invalidate
    // its iterators throughout the lifetime of this StructurePiecesList.
    StructurePiecesList(std::vector<GeneratedPiece>& pieces) {
        this->at = pieces.begin();
        this->end = pieces.end();
    }

    // Returns nullptr if there are no more pieces
    // (in which case it is safe to delete the vector).
    // Do not delete the return pointer; it is managed.
    GeneratedPiece * getNext() {
        if (this->at == this->end) return nullptr;
        GeneratedPiece * val = &(*this->at);
        ++this->at;
        return val;
    }

private:
    std::vector<GeneratedPiece>::iterator at, end;
};

#endif