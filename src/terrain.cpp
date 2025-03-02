#include <mcmapper/terrain.hpp>

#include <mcmapper/noise.hpp>

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

static f64 cavesEntrances(f64 x, f64 y, f64 z) {
    
}
