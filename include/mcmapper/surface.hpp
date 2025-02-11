#ifndef SURFACE_HPP
#define SURFACE_HPP

#include <mcmapper/noises.hpp>

f64 yClampedGradient(f64 y, i32 fromY, i32 toY, f64 fromValue, f64 toValue) {
    f64 delta = (y - fromY) / (toY - fromY);
    if (delta < 0.0) return fromValue;
    if (delta > 1.0) return toValue;
    return lerp(delta, fromValue, toValue);
}

//blending doesn't matter right?
f64 sampleOffset(f64 x, f64 y, f64 z) {

}

#endif