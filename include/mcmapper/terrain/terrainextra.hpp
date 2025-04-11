#ifndef TERRAINEXTRA_HPP
#define TERRAINEXTRA_HPP

#include <mcmapper/terrain/terrain.hpp>

double sampleDensityNether(TerrainGeneratorConfig& config, double x, double y, double z);
double sampleDensityEnd(TerrainGeneratorConfig& config, double x, double y, double z);

#endif