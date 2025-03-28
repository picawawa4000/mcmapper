#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include <mcmapper/rng/noises.hpp>

f64 sampleInitialDensity(Noises& noises, f64 x, f64 y, f64 z);
f64 sampleFinalDensity(Noises& noises, f64 x, f64 y, f64 z);

#endif