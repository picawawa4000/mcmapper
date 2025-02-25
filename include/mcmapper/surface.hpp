#ifndef SURFACE_HPP
#define SURFACE_HPP

#include <mcmapper/noises.hpp>

/*
lv = spaghetti3dRarityNoise.sample(x * 2.0, y, z * 2.0)

lv5 = clamp(lv2 + max(lv3, lv4), -1.0, 1.0)
lv6 = spaghettiRoughnessFunction
lv7 = caveEntranceNoise.sample(x * 0.75, y * 0.5, z * 0.75)
lv8 = lv7 + 0.37 + yClampedGradient(-10, 30, 0.3, 0.0)
cavesEntrances = min(lv8, lv6 + lv5)

base3dNoise = createBase3dNoise(0.25, 0.125, 80.0, 160.0, 8.0)

lv9 = jaggedness * jaggedNoise.halfNegative()
lv10 = initialDensityFunction(factor, depth + lv9)
slopedCheese = lv10 + base3dNoise

lv13 = min(slopedCheese, 5 * cavesEntrances)
// when slopedCheese is in the range, return lv13; otherwise, return caves function
lv14 = rangeChoice(slopedCheese, -1000000, 1.5625, lv13, createCavesFunction(slopedCheese))
finalDensity = min(applySurfaceSlides(lv14), noodleCaves)
*/

#endif