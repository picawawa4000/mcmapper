#ifndef STRUCTURE_HPP
#define STRUCTURE_HPP

#include <mcmapper/rng.hpp>
#include <mcmapper/biomes.hpp>

#include <vector>

enum SpreadType {
    LINEAR,
    TRIANGULAR
};

struct StructurePlacement {
    i32 spacing;
    i32 separation;
    i32 salt;
    SpreadType spreadType;
    const std::vector<Biome> legalBiomes;

    StructurePlacement(i32 spacing, i32 separation, SpreadType spreadType, i32 salt) : spacing(spacing), separation(separation), salt(salt), spreadType(spreadType), legalBiomes({}) {}
    StructurePlacement(i32 spacing, i32 separation, SpreadType spreadType, i32 salt, const std::vector<Biome> legalBiomes) : spacing(spacing), separation(separation), salt(salt), spreadType(spreadType), legalBiomes(legalBiomes) {}

    Pos2D findStart(i64 worldSeed, i32 regionX, i32 regionZ) const {
        CheckedRandom rng((i64)regionX * 341873128712ULL + (i64)regionZ * 132897987541ULL + worldSeed + (i64)this->salt);
        i32 dist = this->spacing - this->separation;

        //I tried to make this part an inline function but it didn't work...
        i32 offsetX, offsetZ;
        switch (this->spreadType) {
            case SpreadType::LINEAR: {
                offsetX = rng.next_i32(dist);
                offsetZ = rng.next_i32(dist);
                break;
            }
            case SpreadType::TRIANGULAR: {
                offsetX = (rng.next_i32(dist) + rng.next_i32(dist)) / 2;
                offsetZ = (rng.next_i32(dist) + rng.next_i32(dist)) / 2;
                break;
            }
            default: throw std::runtime_error("Bad spread type!");
        }

        return Pos2D(regionX * this->spacing + offsetX, regionZ * this->spacing + offsetZ);
    }
};

struct Structure {
    Tag<Biome> validBiomes;

    Structure(Tag<Biome> validBiomes) : validBiomes(validBiomes) {}
};

struct StructureSet {
    std::vector<std::pair<Structure, int>> weightedStructures;
    StructurePlacement placement;

    StructureSet(std::vector<std::pair<Structure, int>> weightedStructures, StructurePlacement placement) : weightedStructures(weightedStructures), placement(placement) {}

    
};

#endif