#include <mcmapper/terrain/terrain.hpp>

#include <mcmapper/rng/noises.hpp>
#include <mcmapper/terrain/internoise.hpp>

static inline std::unique_ptr<InterpolatedNoise> getInterpolatedNoise(u64 seed, Dimension dimension) {
    CheckedRandom rng(seed);
    switch (dimension) {
        case DIM_OVERWORLD: return std::make_unique<InterpolatedNoise>(rng, 0.25, 0.125, 80.0, 160.0, 8.0);
        case DIM_NETHER: return std::make_unique<InterpolatedNoise>(rng, 0.25, 0.375, 80.0, 60.0, 8.0);
        case DIM_END: return std::make_unique<InterpolatedNoise>(rng, 0.25, 0.25, 80.0, 160.0, 4.0);
        default: throw std::runtime_error("Unknown dimension " + std::to_string(dimension) + "!");
    }
}

TerrainGeneratorConfig::TerrainGeneratorConfig(u64 seed, Dimension dimension) {
    this->dimension = dimension;

    this->noises = std::make_shared<Noises>(seed);
    this->interNoise = getInterpolatedNoise(seed, dimension);
    if (dimension == Dimension::DIM_END)
        this->endIslandsNoise = std::make_unique<EndIslandsNoise>(seed);
}

TerrainGeneratorConfig::TerrainGeneratorConfig(std::shared_ptr<Noises> noises, u64 seed, Dimension dimension) {
    this->dimension = dimension;

    this->noises = noises;
    this->interNoise = getInterpolatedNoise(seed, dimension);
    if (dimension == Dimension::DIM_END)
        this->endIslandsNoise = std::make_unique<EndIslandsNoise>(seed);
}

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

static inline f64 initialDensity(f64 factor, f64 depth) {
    f64 f = depth * factor;
    return f > 0. ? f * 4. : f;
}

static f64 slopedCheese(TerrainGeneratorConfig& config, f64 x, f64 y, f64 z) {
    f64 inX = x + config.noises->offset.sample(x, 0, z) * 4.;
    f64 inZ = z + config.noises->offset.sample(z, x, 0) * 4.;

    f64 weirdness = config.noises->weirdness.sample(inX, 0, inZ);
    std::array<f32, 4> params = {
        static_cast<f32>(config.noises->continentalness.sample(inX, 0, inZ)),
        static_cast<f32>(config.noises->erosion.sample(inX, 0, inZ)),
        static_cast<f32>(pvTransform(weirdness)),
        static_cast<f32>(weirdness)
    };
    
    f64 depth = offsetSpline()->sample(params) + yClampedGradient(y, -64, 320, 1.5, -1.5);
    f64 factor = factorSpline()->sample(params);
    f64 jagged = config.noises->jagged.sample(x * 1500., 0, z * 1500.);
    jagged = jagged >= 0. ? jagged : jagged / 2.;
    jagged *= jaggednessSpline()->sample(params);

    f64 density = initialDensity(factor, depth + jagged);
    
    return density + config.interNoise->sample(x, y, z);
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
    // Likely redundant
    f64 yValue = yClampedGradient(y, -4064, 4062, -4064., 4062.);
    f64 noodle = 60 <= yValue && yValue <= 320 ? noises.noodle.sample(x, y, z) : -1;
    f64 noodleThickness = -0.025 * noises.noodle_thickness.sample(x, y, z) - 0.075;
    noodleThickness = -60 <= yValue && yValue <= 320 ? noodleThickness : 0;
    const f64 ridgeMult = 2. + 2. / 3.;
    f64 ridgeA = -60 <= yValue && yValue <= 320 ? noises.noodle_ridge_a.sample(x * ridgeMult, y * ridgeMult, z * ridgeMult) : 0;
    f64 ridgeB = -60 <= yValue && yValue <= 320 ? noises.noodle_ridge_b.sample(x * ridgeMult, y * ridgeMult, z * ridgeMult) : 0;
    f64 f = 1.5 * std::max(std::abs(ridgeA), std::abs(ridgeB));
    return noodle < 0. ? 64. : noodleThickness + f;
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
}

f64 sampleInitialDensity(Noises& noises, f64 x, f64 y, f64 z) {
    f64 inX = x / 4. + noises.offset.sample(x, 0, z) * 4.;
    f64 inZ = x / 4. + noises.offset.sample(z, x, 0) * 4.;

    f64 weirdness = noises.weirdness.sample(x / 4., 0, z / 4.);
    std::array<f32, 4> params = {
        static_cast<f32>(noises.continentalness.sample(x / 4., 0, z / 4.)),
        static_cast<f32>(noises.erosion.sample(x / 4., 0, z / 4.)),
        static_cast<f32>(pvTransform(weirdness)),
        static_cast<f32>(weirdness)
    };
    
    f64 depth = offsetSpline()->sample(params) + yClampedGradient(y, -64, 320, 1.5, -1.5) - 0.50375f;
    f64 factor = factorSpline()->sample(params);

    return surfaceSlides(y, std::clamp(initialDensity(factor, depth) - 0.703125f, -64., 64.));
}

f64 sampleFinalDensity(TerrainGeneratorConfig& config, f64 x, f64 y, f64 z) {
    if (config.dimension != Dimension::DIM_OVERWORLD)
        throw std::runtime_error("Config used for Overwolrd generation initialised for non-Overworld dimension!");
    
    f64 cheese = slopedCheese(config, x, y, z);
#ifndef NDEBUG
    std::cout << "cheese = " << cheese << "\n"
            << "cavesEntrances = " << 5 * cavesEntrances(*config.noises, x, y, z) << "\n"
            << "caves = " << sampleCaves(*config.noises, x, y, z, cheese) << "\n"
            << "cavesNoodle = " << cavesNoodle(*config.noises, x, y, z) << std::endl;
#endif
    f64 g = cheese < 1.5625 ? std::min(cheese, 5 * cavesEntrances(*config.noises, x, y, z)) : sampleCaves(*config.noises, x, y, z, cheese);
    return std::min(cavesNoodle(*config.noises, x, y, z), squeeze(surfaceSlides(y, g) * 0.64));
}
