#ifndef DENSITYFUNCTION_BASE_HPP
#define DENSITYFUNCTION_BASE_HPP

#include <mcmapper/util.hpp>

// Abstract density function type.
// (This simplifies a lot of the code down the line).
struct DensityFunction {
    virtual double operator()(Pos3D pos) = 0;
};

#endif