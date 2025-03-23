#include <mcmapper/terrain/terrain.hpp>

#include <mcmapper/rng/noises.hpp>

static f64 scaleCaves(f64 value) {
    if (value < -0.75) return 0.5;
    if (value < -0.5) return 0.75;
    if (value < 0.5) return 1.;
    if (value < 0.75) return 2.;
    return 3.;
}

static f64 scaleTunnels(f64 value) {
    if (value < -0.5) return 0.75;
    if (value < 0.) return 1.;
    if (value < 0.5) return 1.5;
    return 2.;
}

// If useCaveScale is false, then it scales tunnels instead.
// TYPE_1 = false
static f64 sampleWeirdScaled(f64 x, f64 y, f64 z, f64 input, const DoublePerlinNoise& noise, bool useCaveScale) {
    f64 factor = useCaveScale ? scaleCaves(input) : scaleTunnels(input);
    return factor * std::abs(noise.sample((f64)x / factor, (f64)y / factor, (f64)z / factor));
}

static f64 spaghettiRoughness(Noises& noises, f64 x, f64 y, f64 z) {
    f64 roughness = noises.spaghetti_roughness.sample(x, y, z);
    f64 modulator = noises.spaghetti_roughness_modulator.sample(x, y, z) * -0.1 - 0.05;
    return modulator * (std::abs(roughness) - 0.4);
}

static f64 cavesEntrances(Noises& noises, f64 x, f64 y, f64 z) {
    f64 rarity = noises.spaghetti3d_rarity.sample(x * 2., y, z * 2.);
    f64 thickness = noises.spaghetti3d_thickness.sample(x * -0.065, y * -0.088, z * -0.065);
    f64 spaghetti1 = sampleWeirdScaled(x, y, z, rarity, noises.spaghetti3d_1, false);
    f64 spaghetti2 = sampleWeirdScaled(x, y, z, rarity, noises.spaghetti3d_2, false);
    f64 spaghetti = std::clamp(thickness + std::max(spaghetti1, spaghetti2), -1., 1.);
    f64 roughness = spaghettiRoughness(noises, x, y, z);
    f64 entrance = noises.cave_entrance.sample(x * 0.75, y * 0.5, z * 0.75);
    f64 sum = entrance + 0.37 + yClampedGradient(y, -10, 30, 0.3, 0.);
    return std::min(sum, roughness + spaghetti);
}

static f64 spaghetti2d(Noises& noises, f64 x, f64 y, f64 z) {
    f64 modulator = noises.spaghetti2d_modulator.sample(x * 2., y, z * 2.);
    f64 sample = sampleWeirdScaled(x, y, z, modulator, noises.spaghetti2d, true);
    f64 elevation = 8. * noises.spaghetti2d_elevation.sample(x, 0., z);
    f64 thickness = -0.35 * noises.spaghetti2d_thickness.sample(x * 2., y, z * 2.) - 0.95;
    f64 elevationWithY = std::abs(elevation + yClampedGradient(y, -64, 320, 8.0, -40.0));
    f64 f = elevationWithY + thickness;
    f = f * f * f;
    f64 g = sample + 0.083 * thickness;
    return std::clamp(std::max(f, g), -1., 1.); 
}

static f64 slopedCheese(Noises& noises, f64 x, f64 y, f64 z) {
    throw std::runtime_error("unimplemented function!");
}

static f64 cavesPillars(Noises& noises, f64 x, f64 y, f64 z) {
    f64 pillar = noises.pillar.sample(x * 25., y * 0.3, z * 25.);
    f64 rareness = -noises.pillar_rareness.sample(x, y, z) - 1.;
    f64 thickness = 0.55 * noises.pillar_thickness.sample(x, y, z) + 0.55;
    thickness = thickness * thickness * thickness;
    f64 f = pillar * 2. + rareness;
    return f * thickness;
}

static f64 cavesNoodle(Noises& noises, f64 x, f64 y, f64 z) {
    throw std::runtime_error("unimplemented function!");
}

static f64 sampleCaves(Noises& noises, f64 x, f64 y, f64 z, f64 scheese) {
    f64 spaghetti = spaghetti2d(noises, x, y, z);
    f64 roughness = spaghettiRoughness(noises, x, y, z);
    f64 layer = noises.cave_layer.sample(x, y * 8., z);
    layer = 4. * layer * layer;
    f64 cheese = noises.cave_cheese.sample(x, y * (2./3.), z);
    f64 f = std::clamp(0.27 + cheese, -1., 1.) + std::clamp(1.5 + -0.64 * scheese, 0., 0.5);
    f64 g = f + layer;
    f64 h = std::min(cavesEntrances(noises, x, y, z), std::min(g, spaghetti + roughness));
    f64 pillars = cavesPillars(noises, x, y, z);
    if (pillars < 0.03) return h; // the operation in-game returns -1000000.0 if pillars < 0.03, which is cancelled out by the next max() operation
    return std::max(pillars, h);
}

// minY: -64, maxY: 384, topRelativeMinY: 80, topRelativeMaxY: 64, topDensity: -0.078125, bottomRelativeMinY: 0, bottomRelativeMaxY: 24, bottomDensity: 0.1171875
static inline f64 surfaceSlides(f64 y, f64 density) {
    f64 ret = lerp(yClampedGradient(y, 240, 256, 1., 0.), -0.078125, density);
    return lerp(yClampedGradient(y, -64, -40, 0., 1.), 0.1171875, ret);
;}

f64 sampleFinalDensity(Noises& noises, f64 x, f64 y, f64 z) {
    f64 cheese = slopedCheese(noises, x, y, z);
    f64 f = std::min(cheese, 5 * cavesEntrances(noises, x, y, z));
    f64 g = cheese < 1.5625 ? f : sampleCaves(noises, x, y, z, cheese);
    return std::min(cavesNoodle(noises, x, y, z), surfaceSlides(y, g));
}
