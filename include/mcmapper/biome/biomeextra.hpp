#ifndef BIOMEEXTRA_HPP
#define BIOMEEXTRA_HPP

#include <mcmapper/biome/biomes.hpp>

// Hardcoded for now, but at some point Nether and End biome trees might become a reality.
Biome getNether(double temperature, double humidity);
// See note on getNether(). Additionally, this function does not take into account that
// THE_END should be returned if the distance from the origin is less than 1024 (this is for
// a step further down the chain to implement, so that erosion sampling can be skipped).
Biome getEnd(double erosion);

#endif